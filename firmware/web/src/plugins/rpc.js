/** @file
RPC plugin

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

export default {
  install (Vue, options) {
    const wsUrl = `ws://${document.location.host}/api/v1`
    Vue.rpc = {
      connect () {
        this.ws_ = new WebSocket(wsUrl)
        this.ws_.onopen = () => this.onConnected_()
        this.ws_.onclose = () => this.onDisconnect_()
        this.ws_.onerror = (e) => console.log('onerror', e)
        this.ws_.onmessage = (m) => this.onMessage_(JSON.parse(m.data))
      },

      request (method) {
        if (!this.connected_) throw new Error('Not connected')

        return new Promise((resolve, reject) => {
          const id = this.nextId_++
          this.ws_.send(JSON.stringify({
            jsonrpc: '2.0',
            id,
            method
          }))
          this.pending_[id] = { resolve, reject }
        })
      },

      get state () {
        return this.state_
      },

      $on (...args) {
        return this.eventHub_.$on(...args)
      },
      $off (...args) {
        return this.eventHub_.$off(...args)
      },

      async onConnected_ () {
        this.connected_ = true
        this.nextId_ = 0
        this.state_ = await this.request('state')
        this.eventHub_.$emit('connect', this.state_)
      },
      onDisconnect_ () {
        this.connected_ = false
        for (const p of this.pending_) {
          this.pending_[p].reject('Disconnected')
        }
        this.pending_ = {}
        this.eventHub_.$emit('disconnect')
      },
      onMessage_ (msg) {
        console.log('RPC', msg)
        if (msg['jsonrpc'] !== '2.0') return

        const promise = this.pending_[msg.id]
        if (promise) {
          delete this.pending_[msg.id]
          promise.resolve(msg.result)
        } else if (msg.method === 'update') {
          this.eventHub_.$emit('update', msg.params)
        }
      },

      eventHub_: new Vue(),
      pending_: { },
      state_: { }
    }

    Vue.rpc.connect()
  }
}
