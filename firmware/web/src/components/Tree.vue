<!--
Render an object as a ul tree

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="tree">
    <div class="collapsable" @click="toggle">
      <icon :name="(itemCollapsed) ? 'caret-right' : 'caret-down'"/><span class="key">{{name}}</span>
    </div>
    <template v-if="!itemCollapsed">
      <ul>
        <template v-for="e in entries">
          <li :key=e.key v-if="e.value === null || typeof(e.value) !== 'object'">
            <icon name="square" /><span class="key">{{e.key}}:</span> <span class="value">{{e.value}}</span>
          </li>
          <li :key=e.key v-else>
            <Tree :name=e.key :value=e.value :collapsed="collapsedChildren" />
          </li>
        </template>
      </ul>
    </template>
  </div>
</template>

<script lang="ts">
import { Component, Prop, Vue } from 'vue-property-decorator';
import 'vue-awesome/icons/caret-down';
import 'vue-awesome/icons/caret-right';
import 'vue-awesome/icons/square';

@Component({ name: 'Tree' })
export default class Tree extends Vue {
  @Prop({ type: Boolean, default: false }) public collapsed!: boolean;
  @Prop({ type: Boolean, default: true }) public collapsedChildren!: boolean;
  @Prop({ type: String, default: 'root' }) public name!: string;
  @Prop({ type: Object, required: true }) public value!: any;

  public itemCollapsed = true;

  public mounted() {
    this.itemCollapsed = this.collapsed;
  }

  get entries() {
    if (this.itemCollapsed) { return []; }

    return Object.entries(this.value).map((e) => {
      return { key: e[0], value: e[1] };
    }).sort((lhs, rhs) => String(lhs.key).localeCompare(rhs.key));
  }

  public toggle() {
    this.itemCollapsed = !this.itemCollapsed;
  }
}
</script>

<style lang="scss" scoped>
ul {
  padding-left: 14px;
}
li {
  list-style: none;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

div.collapsable {
  cursor: pointer;
}

span.key {
  color: #222;
  font-weight: bold;
}

.fa-icon {
  margin-right: 4px;
  text-anchor: center;
  vertical-align: text-bottom;
  width: 8px;
}
li > .fa-icon {
  color: #777;
}
</style>
