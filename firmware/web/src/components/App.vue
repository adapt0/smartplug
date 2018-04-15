<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div id="app">
    <connection-alert :value="$store.state.Rpc.connected">
    </connection-alert>
    <div class="body">
      <AppSidebar />
      <router-view class="route" />
    </div>
    <div class="connection-overlay" v-bind:class="{ offline: $store.state.Rpc.connected === false }"></div>
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
#app > .body > .route {
  overflow: scroll;
  flex: 1;
  min-width: 0;
}
.route .header {
  padding-left: 1em;
  padding-top: 0.4em;
  height: 3em;
}
.route .content {
  padding: 1em;
}
.route .header + .content {
  padding-top: 0;
}

.route .tabs .card-header {
  /*background-color: #343a40;*/
  padding-top: 0.4rem;
}

.nav-item .nav-link {
  color: #333;
}
.nav-item .nav-link .feather {
  color: #999;
}
.nav-item .nav-link.disabled {
  color: #aaa;
}
.nav-item .nav-link.active {
  color: #007bff;
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
