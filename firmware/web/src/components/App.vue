<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div id="app">
    <connection-alert :value="!$store.state.Rpc.connected">
    </connection-alert>
    <div class="body">
      <AppSidebar />
      <div class="content">
        <router-view/>
      </div>
    </div>
    <div class="connection-overlay" v-bind:class="{ offline: !$store.state.Rpc.connected }"></div>
  </div>
</template>

<script>
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'
import AppSidebar from '@/components/AppSidebar'
import ConnectionAlert from '@/components/shared/ConnectionAlert'

export default {
  name: 'App',
  components: {
    AppSidebar,
    ConnectionAlert
  }
}
</script>

<style>
html, body, #app {
  height: 100%;
  width: 100%;
  overflow: hidden;
}

#app {
  display: flex;
  flex-direction: column;
}

#app > .body {
  flex: 1;
  display: flex;
}
#app > .body > .content {
  overflow: scroll;
  flex: 1;
  min-width: 0;
  padding: 1em;
}

#app > .connection-overlay {
  opacity: 0;
  pointer-events: none;
  position: absolute;
  width: 100%;
  height: 100%;
}
#app > .connection-overlay.offline {
  background-color: #000;
  opacity: 0.5;
  transition: opacity 2s ease 1s;
}
</style>
