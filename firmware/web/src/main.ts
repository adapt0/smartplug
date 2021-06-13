/** @file
 * Application entry point
 *
 * \copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
 * Licensed under the MIT License. Refer to LICENSE file in the project root.
 */
import Vue from 'vue';
import App from '@/views/App.vue';
import BootstrapVue from 'bootstrap-vue';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { library } from '@fortawesome/fontawesome-svg-core';
import router from '@/router';
import store from '@/store';
import VueResource from 'vue-resource';
import {
  faBug, faCaretDown, faCaretRight, faCog, faHome, faInfoCircle, faSpinner, faSquare,
} from '@fortawesome/free-solid-svg-icons';

Vue.config.productionTip = false;

library.add(
  faBug, faCaretDown, faCaretRight, faCog, faHome, faInfoCircle, faSpinner, faSquare,
);
Vue.component('font-awesome-icon', FontAwesomeIcon);

Vue.use(BootstrapVue);
Vue.use(VueResource);

new Vue({
  router,
  store,
  render: (h) => h(App),
}).$mount('#app');
