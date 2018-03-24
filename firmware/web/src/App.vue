<template>
  <div id='app'>
    <connection-alert :value='!$store.getters.rpcConnected'>
    </connection-alert>
    <div class='body' v-bind:class='{ offline: !$store.getters.rpcConnected }' >
      <Sidebar />
      <div class='content'>
        <router-view/>
      </div>
    </div>
  </div>
</template>

<script>
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'
import ConnectionAlert from './components/ConnectionAlert'
import Sidebar from './components/Sidebar'

export default {
  name: 'App',
  components: {
    ConnectionAlert,
    Sidebar
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
#app > .body.offline {
  opacity: 0.4;
  transition: opacity 2s ease 1s;
}
</style>
