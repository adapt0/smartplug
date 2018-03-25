<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="about">
    <div class="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pb-2 mb-3 border-bottom">
      <h1 class="h2">About</h1>
    </div>
    <h5>SmartPlug {{$store.state.Rpc.data.version}}</h5>
    <ul>
      <li>GitHub URL</li>
      <li>Libraries</li>
    </ul>
  </div>
</template>

<script>
export default {
  name: 'About'
}
</script>

<style scoped>
</style>
