<template>
  <div class="index">
    <h2>{{ $store.state.test.int }}</h2>
    <button v-on:click="$store.dispatch('test')">Add 1</button>
  </div>
</template>

<script>
export default {
  name: 'Index'
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
