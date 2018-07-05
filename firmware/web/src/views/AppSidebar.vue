<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="d-flex flex-column sidebar">
    <b-nav vertical>
      <b-nav-text><router-link to="/"><h5>SmartPlug</h5></router-link></b-nav-text>
      <b-nav-item to="/home"><icon name="home"/>Home</b-nav-item>

      <b-nav-item to="/settings"><icon name="cog"/>Settings</b-nav-item>

    <router-link tag="div" to="/settings">
      <b-nav vertical>
        <b-nav-item to="/settings/general">General</b-nav-item>
        <b-nav-item to="/settings/network">Network</b-nav-item>
        <b-nav-item to="/settings/security">Security</b-nav-item>
        <b-nav-item to="/settings/upgrade">Upgrade</b-nav-item>
      </b-nav>
    </router-link>

      <b-nav-item to="/developer"><icon name="bug"/>Developer</b-nav-item>
      <b-nav-item to="/about"><icon name="info-circle"/>About</b-nav-item>
    </b-nav>
    <div class="mt-auto text-center p-4">
      <bootstrap-toggle v-model="relay" :disabled="!$store.state.Rpc.connected"></bootstrap-toggle>
    </div>
  </div>
</template>

<script>
import BootstrapToggle from '@/components/BootstrapToggle'
import 'vue-awesome/icons/bug'
import 'vue-awesome/icons/cog'
import 'vue-awesome/icons/home'
import 'vue-awesome/icons/info-circle'

export default {
  components: {
    BootstrapToggle
  },
  computed: {
    relay: {
      get () {
        return Boolean(this.$store.state.Rpc.data.relay)
      },
      set (state) {
        const relay = this.$store.state.Rpc.data.relay
        if (this.$store.state.Rpc.connected && (relay !== null) && (typeof relay !== 'undefined')) {
          console.log('set relay', state, this.$store.state.Rpc.data.relay)
          this.$store.dispatch('Rpc/relay', state)
        }
      }
    }
  }
}
</script>

<style scoped>
.sidebar {
  background-color: #f8f9fa;
  box-shadow: inset -1px 0 0 rgba(0, 0, 0, .1);
}

.sidebar .navbar-text {
  background-color: #343a40;
  padding-top: 0.75rem;
  padding-bottom: 0.75rem;
  text-align: center;
}
.sidebar .navbar-text h5,
.sidebar .navbar-text a {
  color: #fff;
  font-weight: bold;
  text-decoration: none;
  margin: 0;
}

.sidebar .nav-link {
  font-weight: 500;
  display: flex;
  align-items: center;
}
.sidebar .nav-link .fa-icon {
  margin-right: 8px;
  width: 14px;
}
.sidebar .nav-link .feather {
  margin-right: 4px;
}
.sidebar .nav-link.active {
  background-color: rgba(0, 0, 0, 0.05);
}

.sidebar .nav .nav .nav-link {
  display: none;
  font-weight: 1500;
  font-size: 85%;
  padding: 0.4rem 0 0.4rem 2.5rem;
}
.sidebar .nav .router-link-active .nav-link {
  background-color: rgba(0, 0, 0, 0.05);
  display: block;
}
.sidebar .nav .nav .nav-link.active {
  background-color: rgba(0, 0, 0, 0.08);
}

.sidebar .nav-item .nav-link:hover {
  background-color: rgba(0, 0, 0, 0.08);
}

.sidebar .nav-item .nav-link:hover .feather,
.sidebar .nav-item .nav-link.active .feather {
  color: inherit;
}
</style>
