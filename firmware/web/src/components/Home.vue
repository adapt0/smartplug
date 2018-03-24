<template>
  <div class="index">
    <div class="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pb-2 mb-3 border-bottom">
      <h1 class="h2">Dashboard</h1>
      <b-button-toolbar key-nav>
        <b-button-group class="mr-2">
          <b-button class="btn-sm" variant="outline-secondary">Share</b-button>
          <b-button class="btn-sm" variant="outline-secondary">Export</b-button>
        </b-button-group >
        <b-dropdown text="This week" class="mx-1" variant="outline-secondary">
          <b-dropdown-item>calendar</b-dropdown-item>
        </b-dropdown>
      </b-button-toolbar>
    </div>

    <Chart titleText="Wattage" v-bind:in="$store.state.Rpc.wattage"></Chart>

    <div>
      <h2>{{ $store.state.Rpc.data.voltage }}V</h2>

      <h2>int: {{ $store.state.Rpc.data.test.int }}</h2>
      <div>Connected: {{ $store.state.Rpc.connected }}</div>
      <button v-on:click="$store.dispatch('Rpc/test')">Add 1</button>
    </div>

    <div>
      <h2>Relay: y{{$store.state.Rpc.data.relay}}</h2>
    </div>
  </div>
</template>

<script>
import Chart from '@/components/chart'

export default {
  name: 'Home',
  components: {
    Chart
  }
}
</script>

<style scoped>
</style>
