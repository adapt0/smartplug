/** @file
Application entry point

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from '@/components/App'
import BootstrapVue from 'bootstrap-vue'
import Icon from 'vue-awesome/components/Icon'
import router from '@/router'
import Rpc from '@/plugins/rpc'
import store from '@/store'

Vue.config.productionTip = false

// bootstrap
Vue.use(BootstrapVue)

// vue-awesome icon component
Vue.component('icon', Icon)

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
  template: '<App/>',
  mounted () {
    this.timerId_ = setInterval(() => {
      if (Vue.rpc.connected) {
        const power = store.state.data.power
        if (typeof power !== 'undefined') {
          store.commit('wattage', power)
        }
      }
    }, 1000)
  },
  beforeDestroy () {
    if (this.timerId_) clearInterval(this.timerId_)
  }
})
