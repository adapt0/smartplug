<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="connection alert d-flex justify-content-between" :class="[ disconnected ? 'alert-danger' : 'alert-success', { active: disconnected }]" role="alert">
    <div class="container">
      <div class="row">
        <template v-if="disconnected">
          <div class="col">&nbsp;</div>
          <div class="col-md-auto text-center">
            Connection lost. Attempting to reconnect...
          </div>
          <div class="col text-right">
            <icon class="align-text-bottom" name="spinner" pulse />
          </div>
        </template>
        <template v-else>
          <div class="col-sm">&nbsp;</div>
          <div class="col-md-auto text-center">
            Connection reestablished!
          </div>
          <div class="col-sm">&nbsp;</div>
        </template>
      </div>
    </div>
 </div>
</template>

<script lang="ts">
import { Component, Prop, Vue } from 'vue-property-decorator';
import 'vue-awesome/icons/spinner';

@Component
export default class ConnectionAlert extends Vue {
  @Prop({ type: Boolean, default: true }) public value!: boolean;

  get disconnected() {
    return this.value === false;
  }
}
</script>

<style lang="scss" scoped>
.connection.alert {
  position: absolute;
  left: 50%;
  transform: translate(-50%, 0);
  min-width: 40%;

  border-top-left-radius: 0;
  border-top-right-radius: 0;
  margin: 0;
  z-index: 1000;

  pointer-events: none;

  opacity: 0;
  top: -100%;
  transition: opacity, top;
  transition-delay: 1s;
  transition-duration: 2s;

  &.active {
    opacity: 1;
    top: 0%;
    transition: opacity, top;
    transition-duration: 2s;
  }
}
</style>
