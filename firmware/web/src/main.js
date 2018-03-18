// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import Vuex from 'vuex'
import App from './App'
import router from './router'
import Rpc from './plugins/rpc'
import { mergeDeep } from './helpers/mergeDeep.js'

Vue.config.productionTip = false

// Vuex state management
Vue.use(Vuex)
const store = new Vuex.Store({
  state: {
    sys: { },
    test: { }
  },
  getters: {
    rpcConnected () {
      return Vue.rpc.connected
    }
  },
  mutations: {
    stateNew (state, results) {
      Object.assign(state, results)
    },
    stateUpdate (state, results) {
      mergeDeep(state, results)
    }
  },
  actions: {
    test (/* context */) {
      return Vue.rpc.request('test')
    }
  }
})

// WebSocket RPC
Vue.use(Rpc, '/api/v1')
Vue.rpc.on('connect', (state) => {
  store.commit('stateNew', state)
})
Vue.rpc.on('disconnect', () => {
  // console.log(store, store.rpcConnected, Vue.rpc.connected)
})
Vue.rpc.on('update', (state) => {
  store.commit('stateUpdate', state)
})

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  store,
  components: { App },
  template: '<App/>'
})
