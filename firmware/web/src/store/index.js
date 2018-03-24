/** @file
Vuex store

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'
import Vuex from 'vuex'
import { mergeDeep } from '@/helpers/mergeDeep.js'

Vue.use(Vuex)

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

export default new Vuex.Store({
  state: {
    wattage: placeholderWattage(),
    data: {
      relay: null,
      sys: { },
      test: { }
    }
  },
  getters: {
    rpcConnected () {
      return Boolean(Vue.rpc.connected)
    }
  },
  mutations: {
    stateNew (state, results) {
      Object.assign(state.data, results)
    },
    stateUpdate (state, results) {
      mergeDeep(state.data, results)
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
      return Vue.rpc.request('relay', state)
    },
    test (/* context */) {
      return Vue.rpc.request('test')
    }
  }
})
