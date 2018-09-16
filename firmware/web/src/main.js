/** @file
Application entry point

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'
import App from '@/views/App.vue'
import BootstrapVue from 'bootstrap-vue'
import Icon from 'vue-awesome/components/Icon.vue'
import router from '@/router'
import store from '@/store'
import VueResource from 'vue-resource'

Vue.config.productionTip = false

Vue.use(BootstrapVue)
Vue.use(VueResource)
Vue.component('icon', Icon)

new Vue({
  router,
  store,
  render: (h) => h(App),
}).$mount('#app');
