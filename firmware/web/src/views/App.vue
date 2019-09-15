<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div id="app">
    <connection-alert :value="$store.state.Rpc.connected" />
    <div class="body">
      <AppSidebar />
      <router-view class="route" />
    </div>
    <div class="connection-overlay" :class="{ offline: $store.state.Rpc.connected === false }"></div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import 'bootstrap/dist/css/bootstrap.css';
import 'bootstrap-vue/dist/bootstrap-vue.css';
import AppSidebar from '@/views/AppSidebar.vue';
import ConnectionAlert from '@/components/ConnectionAlert.vue';

@Component({ components: { AppSidebar, ConnectionAlert } })
export default class App extends Vue { }
</script>

<style lang="scss">
html, body, #app {
  height: 100%;
  width: 100%;
  overflow: hidden;
}

#app {
  display: flex;
  flex-direction: column;

  & > .body {
    flex: 1;
    display: flex;
    max-height: 100vh;

    & > .route {
      overflow: auto;
      flex: 1;
      min-width: 0;
    }
  }
}

.route {
  .header {
    padding-left: 1em;
    padding-top: 0.4em;
    height: 3em;
  }
  .content {
    padding: 1em;
  }
  .header + .content {
    padding-top: 0;
  }

  .tabs .card-header {
    /*background-color: #343a40;*/
    padding-top: 0.4rem;
  }
}

.nav-item .nav-link {
  color: #333;

  .feather {
    color: #999;
  }
  &.disabled {
    color: #aaa;
  }
  &.active {
    color: #007bff;
  }
}

#app > .connection-overlay {
  opacity: 0;
  pointer-events: none;
  position: absolute;
  width: 100%;
  height: 100%;

  &.offline {
    background-color: #000;
    opacity: 0.5;
    transition: opacity 2s ease 1s;
  }
}
</style>
