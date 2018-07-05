/** @file
Vue-router

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'
import Router from 'vue-router'
import About from '@/views/About'
import Developer from '@/views/Developer'
import Home from '@/views/Home'
import SettingsGeneral from '@/views/settings/General'
import SettingsNetwork from '@/views/settings/Network'
import SettingsSecurity from '@/views/settings/Security'
import SettingsUpgrade from '@/views/settings/Upgrade'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      redirect: '/home'
    },
    {
      path: '/about',
      name: 'about',
      component: About
    },
    {
      path: '/developer',
      name: 'developer',
      component: Developer
    },
    {
      path: '/home',
      name: 'home',
      component: Home
    },
    {
      path: '/settings',
      redirect: '/settings/general'
    },
    {
      path: '/settings/general',
      component: SettingsGeneral
    },
    {
      path: '/settings/network',
      component: SettingsNetwork
    },
    {
      path: '/settings/security',
      component: SettingsSecurity
    },
    {
      path: '/settings/upgrade',
      component: SettingsUpgrade
    }
  ]
})
