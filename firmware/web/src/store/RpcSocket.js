/** @file
RPC Socket - Handles JSON-RPC over WebSockets

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'

export default class {
  constructor (path = '') {
    this.heartbeatInterval_ = 3000 // ms between heartbeats
    this.url_ = `ws://${document.location.host}${path}`

    this.eventHub_ = new Vue({
      data: {
        connected: true // initially assumes we have a connection
      }
    })

    this.pending_ = { }
    this.reconnectDelay_ = 0
    this.state_ = { }
  }

  connect () {
    console.log(`connecting to ${this.url_} ...`)
    this.ws_ = new WebSocket(this.url_)
    this.ws_.onopen = () => this.onConnected_()
    this.ws_.onclose = () => this.onDisconnect_()
    this.ws_.onerror = (...args) => this.onError_(...args)
    this.ws_.onmessage = (m) => this.onMessage_(JSON.parse(m.data))
  }

  request (method, params) {
    if (!this.connected_) throw new Error('Not connected')

    return new Promise((resolve, reject) => {
      const id = this.nextId_++
      this.ws_.send(JSON.stringify({
        jsonrpc: '2.0',
        id,
        method,
        params
      }))
      this.pending_[id] = { resolve, reject }
    })
  }

  get connected () {
    return this.eventHub_.connected
  }
  get state () {
    return this.state_
  }

  on (...args) {
    return this.eventHub_.$on(...args)
  }
  off (...args) {
    return this.eventHub_.$off(...args)
  }

  async onConnected_ () {
    console.log('connected')
    this.connected_ = true
    this.reconnectDelay_ = 0
    this.eventHub_.connected = true
    this.nextId_ = 0
    this.beginHeartbeat_()
    this.state_ = await this.request('state')
    this.eventHub_.$emit('connect', this.state_)
  }
  onDisconnect_ () {
    // skip if reconnect is already in progress
    if (!this.connected_ && this.timerId_) return

    console.log('disconnected')
    this.eventHub_.connected = false
    this.connected_ = false

    if (this.heartbeatTimer_) {
      clearTimeout(this.heartbeatTimer_)
      delete this.heartbeatTimer_
    }

    // notify pending requests
    Object.keys(this.pending_).forEach((k) => {
      this.pending_[k].reject('Disconnected')
    })
    this.pending_ = {}
    this.eventHub_.$emit('disconnect')

    // delay before retrying connection
    this.reconnectDelay_ = Math.min(Math.max(500, this.reconnectDelay_ * 2), 5000)
    this.timerId_ = setTimeout(() => {
      delete this.timerId_
      this.connect()
    }, this.reconnectDelay_)
  }
  onError_ (e) {
    console.log('onerror', e)
  }
  onMessage_ (msg) {
    console.log('RPC', msg)
    if (msg['jsonrpc'] !== '2.0') return

    // received data on socket, so we can delay any heartbeat
    this.sawActivity_ = true

    // notify any pending request
    const promise = this.pending_[msg.id]
    if (promise) {
      delete this.pending_[msg.id]
      if (msg.error) {
        promise.reject(msg.error)
      } else {
        promise.resolve(msg.result)
      }
    }

    // non-solicited update
    if (msg.method === 'update') {
      this.eventHub_.$emit('update', msg.params)
    }
  }

  beginHeartbeat_ () {
    this.heartbeatTimer_ = setTimeout(async () => {
      if (!this.connected_) return

      // don't send heartbeat if we're receiving other responses
      if (this.sawActivity_) {
        this.sawActivity_ = false
        this.beginHeartbeat_()
        return
      }

      //
      try {
        await Promise.race([
          new Promise((resolve, reject) => setTimeout(() => reject(new Error('timed out')), 3000)),
          this.request('ping')
        ])
        this.beginHeartbeat_()
      } catch (e) {
        console.error('heartbeat', e)
        this.ws_.close()
        delete this.ws_
        this.onDisconnect_()
      }
    }, this.heartbeatInterval_)
  }
}
