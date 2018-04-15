<!--
Render an object as a ul tree

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="tree">
    <div class="collapsable" v-on:click="toggle">
      <icon v-bind:name="(itemCollapsed) ? 'caret-right' : 'caret-down'"/><span class="key">{{name}}</span>
    </div>
    <template v-if="!itemCollapsed">
      <ul>
        <template v-for="e in entries">
          <li :key=e.key v-if="e.value === null || typeof(e.value) !== 'object'"><icon name="square" /><span class="key">{{e.key}}:</span> <span class="value">{{e.value}}</span></li>
          <li :key=e.value v-else><Tree :name=e.key :value=e.value :collapsed="collapsedChildren" /></li>
        </template>
      </ul>
    </template>
  </div>
</template>

<script>
import 'vue-awesome/icons/caret-down'
import 'vue-awesome/icons/caret-right'
import 'vue-awesome/icons/square'

export default {
  name: 'Tree',
  data () {
    return {
      itemCollapsed: this.collapsed
    }
  },
  props: {
    collapsed: {
      type: Boolean,
      default: false
    },
    collapsedChildren: {
      type: Boolean,
      default: true
    },
    name: {
      type: String,
      default: 'root'
    },
    value: {
      type: Object,
      required: true
    }
  },
  computed: {
    entries () {
      return Object.entries(this.value).map((e) => {
        return { key: e[0], value: e[1] }
      }).sort((lhs, rhs) => String(lhs.key).localeCompare(rhs.key))
    }
  },
  methods: {
    toggle () {
      this.itemCollapsed = !this.itemCollapsed
    }
  }
}
</script>

<style scoped>
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
span.value {
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
