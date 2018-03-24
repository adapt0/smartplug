/** @file
Vue-router

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */

import Vue from 'vue'
import Router from 'vue-router'
import About from '@/components/About'
import Developer from '@/components/Developer'
import Home from '@/components/Home'
import Settings from '@/components/Settings'

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
      name: 'settings',
      component: Settings
    }
  ]
})
