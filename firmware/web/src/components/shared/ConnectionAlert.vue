<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="connection alert d-flex justify-content-between" v-bind:class="[ value ? 'alert-danger' : 'alert-success', { active: value }]" role="alert">
    <template v-if="value">
      <div>Connection lost. Attempting to reconnect...</div>
      <div class="pl-4"><icon name="spinner" pulse /></div>
    </template>
    <template v-else>
      <div>Connection reestablished!</div>
    </template>
  </div>
</template>

<script>
import 'vue-awesome/icons/spinner'

export default {
  props: {
    value: {
      type: Boolean,
      default: false
    }
  }
}
</script>

<style scoped>
.connection.alert {
  position: absolute;
  left: 50%;
  transform: translate(-50%, 0);
  min-width: 40%;

  border-top-left-radius: 0;
  border-top-right-radius: 0;
  text-align: center;
  z-index: 1000;

  opacity: 0;
  top: -100%;
  transition: opacity, top;
  transition-delay: 1s;
  transition-duration: 2s;
}
.connection.alert.active {
  opacity: 1;
  top: 0%;
  transition: opacity, top;
  transition-duration: 2s;
}
</style>
