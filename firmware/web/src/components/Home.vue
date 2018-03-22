<template>
  <div class="index">
    <h3>Dashboard</h3>

    <h2>{{ $store.state.data.test.int }}</h2>
    <div>Connected: {{ $store.getters.rpcConnected }}</div>
    <button v-on:click="$store.dispatch('test')">Add 1</button>

    <Chart titleText='Wattage' v-bind:in='$store.state.wattage'></Chart>
  </div>
</template>

<script>
import Chart from './chart'

export default {
  name: 'Home',
  components: {
    Chart
  }}
</script>

<style scoped>
</style>
