<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="d-flex flex-column sidebar">
    <div>
      <b-nav vertical>
        <b-nav-text><router-link to="/"><h5>SmartPlug</h5></router-link></b-nav-text>
        <b-nav-item to="home"><icon name="home"/>Home</b-nav-item>
        <b-nav-item to="settings"><icon name="cog"/>Settings</b-nav-item>
        <b-nav-item to="developer"><icon name="bug"/>Developer</b-nav-item>
        <b-nav-item to="about"><icon name="info-circle"/>About</b-nav-item>
      </b-nav>
    </div>
    <div class="mt-auto text-center p-4">
      <bootstrap-toggle v-model="relay" :disabled="!$store.state.Rpc.connected"></bootstrap-toggle>
    </div>
  </div>
</template>

<script>
import BootstrapToggle from '@/components/shared/BootstrapToggle'
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
  box-shadow: inset -1px 0 0 rgba(0, 0, 0, .1);
}

.sidebar .navbar-text {
  text-align: center;
}
.sidebar .navbar-text h5,
.sidebar .navbar-text a {
  color: #000;
  text-decoration: none;
  margin: 0;
}

.sidebar .nav-link {
  font-weight: 500;
  color: #333;
  display: flex;
  align-items: center;
  padding-right: 2em;
}

.sidebar .nav-link .fa-icon {
  margin-right: 8px;
  width: 14px;
}

.sidebar .nav-link .feather {
  margin-right: 4px;
  color: #999;
}

.sidebar .nav-link.disabled {
  color: #aaa;
}

.sidebar .nav-link.active {
  color: #007bff;
  background-color: rgba(0, 0, 0, 0.05);
}

.sidebar .nav-link:hover .feather,
.sidebar .nav-link.active .feather {
  color: inherit;
}
</style>
