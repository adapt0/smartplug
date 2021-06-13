<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="d-flex flex-column sidebar">
    <b-nav vertical class="flex-nowrap">
      <b-nav-text><router-link to="/"><h5>SmartPlug</h5></router-link></b-nav-text>
      <b-nav-item to="/home"><font-awesome-icon icon="home" />Home</b-nav-item>

      <b-nav-item to="/settings"><font-awesome-icon icon="cog" />Settings</b-nav-item>

      <router-link tag="div" to="/settings">
        <b-nav vertical>
          <b-nav-item to="/settings/general">General</b-nav-item>
          <b-nav-item to="/settings/network">Network</b-nav-item>
          <b-nav-item to="/settings/security">Security</b-nav-item>
          <b-nav-item to="/settings/upgrade">Upgrade</b-nav-item>
        </b-nav>
      </router-link>

      <b-nav-item to="/developer"><font-awesome-icon icon="bug" />Developer</b-nav-item>
      <b-nav-item to="/about"><font-awesome-icon icon="info-circle" />About</b-nav-item>
    </b-nav>
    <div class="mt-auto text-center p-4">
      <bootstrap-toggle v-model="relay" :disabled="!$store.state.Rpc.connected"></bootstrap-toggle>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import BootstrapToggle from '@/components/BootstrapToggle.vue';

@Component({ components: { BootstrapToggle } })
export default class AppSidebar extends Vue {
  get relay() {
    return Boolean(this.$store.state.Rpc.data.relay);
  }
  set relay(state) {
    const relay = this.$store.state.Rpc.data.relay;
    if (this.$store.state.Rpc.connected && (relay !== null) && (typeof relay !== 'undefined')) {
      // console.log('set relay', state, this.$store.state.Rpc.data.relay);
      this.$store.dispatch('Rpc/relay', state);
    }
  }
}
</script>

<style lang="scss" scoped>
.sidebar {
  background-color: #f8f9fa;
  box-shadow: inset -1px 0 0 rgba(0, 0, 0, .1);

  .navbar-text {
    background-color: #343a40;
    padding-top: 0.75rem;
    padding-bottom: 0.75rem;
    text-align: center;

    h5, a {
      color: #fff;
      font-weight: bold;
      text-decoration: none;
      margin: 0;
    }
  }

  .nav-link {
    font-weight: 500;
    display: flex;
    align-items: center;

    [role="img"] {
      margin-right: 8px;
      width: 14px;
    }
    .feather {
      margin-right: 4px;
    }
    &.active {
      background-color: rgba(0, 0, 0, 0.05);
    }
  }

  .nav .nav .nav-link {
    display: none;
    font-weight: 1500;
    font-size: 85%;
    padding: 0.4rem 0 0.4rem 2.5rem;
  }
  .nav .router-link-active .nav-link {
    background-color: rgba(0, 0, 0, 0.05);
    display: block;
  }
  .nav .nav .nav-link.active {
    background-color: rgba(0, 0, 0, 0.08);
  }

  .nav-item {
    .nav-link:hover {
      background-color: rgba(0, 0, 0, 0.08);
    }

    .nav-link:hover .feather,
    .nav-link.active .feather {
      color: inherit;
    }
  }
}
</style>
