<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="about">
    <div class="header d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center border-bottom">
      <h1 class="h2">About</h1>
    </div>
    <div class="content">
      <h5>SmartPlug {{$store.state.Rpc.data.version}}</h5>
      <ul>
        <li>GitHub URL</li>
        <li>Libraries</li>
      </ul>
    </div>
  </div>
</template>

<script>
export default {
  name: 'About'
}
</script>

<style scoped>
</style>
