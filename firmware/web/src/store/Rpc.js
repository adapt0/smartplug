/** @file
RPC store interface

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import RpcSocket from '@/store/RpcSocket'
import { mergeDeep } from '@/helpers/mergeDeep.js'

/// populate some default placeholder wattage readings
const placeholderWattage = () => {
  const now = new Date()
  const dataLength = 60
  return Array(dataLength).fill(now.getTime()).map((v, i) => {
    return [
      v - (dataLength - i) * 1000,
      null
    ]
  })
}

// Vuex store attachment
export default function (store) {
  let lastGitRevision = null

  console.log('RpcInit')

  // WebSocket RPC
  const rpcSocket = new RpcSocket('/api/v1')
  rpcSocket.on('connect', (data) => {
    store.commit('Rpc/connectionState', data)

    // force a reload if the remote git revision has changed since our last connection
    if (lastGitRevision && lastGitRevision !== data.gitRev) {
      console.log('*** RPC forcing reload')
      window.location.reload(true)
    }
    lastGitRevision = data.gitRev

    // start logging Wattage
    const timerId = setInterval(() => {
      if (!rpcSocket.connected) {
        clearInterval(timerId)
        return
      }
      const power = store.state.Rpc.data.power
      if (typeof power !== 'undefined') {
        store.commit('Rpc/wattage', power)
      }
    }, 1000)
  })
  rpcSocket.on('disconnect', () => {
    store.commit('Rpc/connectionState', false)
    // console.log(store, store.rpcConnected, Vue.rpc.connected)
  })
  rpcSocket.on('update', (data) => {
    store.commit('Rpc/connectionUpdate', data)
  })

  // begin establishing a connection
  rpcSocket.connect()

  // bind our RPC module
  store.registerModule('Rpc', {
    namespaced: true,
    state: {
      connected: null, // assumes we're initially connected
      wattage: placeholderWattage(),
      data: {
        relay: null,
        sys: {
          net: {
            cur: {
              ipv4Address: '',
              ipv4Subnet: '',
              ipv4Gateway: '',
              ipv4Dns1: '',
              ipv4Dns2: ''
            },
            dhcp: true,
            ipv4Address: '',
            ipv4Subnet: '',
            ipv4Gateway: '',
            ipv4Dns1: '',
            ipv4Dns2: '',
            hostname: 'hostname',
            ssid: 'ssid'
          }
        },
        test: { },
        version: ''
      }
    },
    mutations: {
      connectionState (state, data) {
        if (data === false) {
          state.connected = false
        } else {
          state.connected = true
          Object.assign(state.data, data)
        }
      },
      connectionUpdate (state, data) {
        console.log('connectionUpdate', state, data)
        mergeDeep(state.data, data)
      },
      wattage (state, power) {
        state.wattage = [
          ...state.wattage.slice(1),
          [ new Date(), power ]
        ]
      }
    },
    actions: {
      relay (context, state) {
        return rpcSocket.request('relay', state)
      },
      network (context, state) {
        return rpcSocket.request('network', state)
      },
      test (/* context */) {
        return rpcSocket.request('test')
      }
    }
  })
}
