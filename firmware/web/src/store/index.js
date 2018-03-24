/** @file
Vuex store

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'
import Vuex from 'vuex'
import Rpc from '@/store/Rpc'

Vue.use(Vuex)

export default new Vuex.Store({
  plugins: [Rpc]
})
