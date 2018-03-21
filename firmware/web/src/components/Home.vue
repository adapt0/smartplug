<template>
  <div class="index">
    <h3>Dashboard</h3>

    <h2>{{ $store.state.test.int }}</h2>
    <div>Connected: {{ $store.getters.rpcConnected }}</div>
    <button v-on:click="$store.dispatch('test')">Add 1</button>
  </div>
</template>

<script>
export default {
  name: 'Home'
}
</script>

<style scoped>
</style>
