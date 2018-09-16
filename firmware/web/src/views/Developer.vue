<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="index">
    <div class="header d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center">
      <h1 class="h2">Developer</h1>
    </div>

    <div class="content row">
      <div class="col-sm-6">
        <div class="card">
          <div class="card-body">
            <Tree name="Rpc.data" :value="$store.state.Rpc.data" :collapsedChildren=false />
          </div>
        </div>
      </div>
      <div class="col-sm-6">
        <div class="card">
          <div class="card-body">
            <div>Connected: {{ $store.state.Rpc.connected }}</div>
            <button v-on:click="$store.dispatch('Rpc/test')">Add 1</button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import Tree from '@/components/Tree.vue';

@Component({ components: { Tree } })
export default class Developer extends Vue { }
</script>

<style lang="scss" scoped>
</style>
