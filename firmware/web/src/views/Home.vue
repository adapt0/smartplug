<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="index">
    <div class="header d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center">
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

    <div class="content">
      <Chart titleText="Wattage" v-bind:in="$store.state.Rpc.wattage"></Chart>

      <div class="d-flex">
        <h2>{{ $store.state.Rpc.data.power }}W</h2>
        <h2 class="pl-2">{{ $store.state.Rpc.data.voltage }}V</h2>
      </div>
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
