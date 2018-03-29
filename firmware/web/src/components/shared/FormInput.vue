<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <b-form-group :label="label" :label-for="id" :invalid-feedback="(required || model.length) ? valid : ''" :state="false" :required="required" horizontal>
    <b-form-input v-model="model" :id="id" :placeholder="placeholder" :readonly="readonly"/>
  </b-form-group>
</template>

<script>
export default {
  props: {
    id: { type: String },
    label: { type: String },
    value: { type: String },
    pattern: { type: String },
    invalidMessage: { type: String },
    placeholder: { type: String },
    readonly: { type: Boolean, default: false },
    required: { type: Boolean, default: false }
  },
  computed: {
    model: {
      get () {
        return this.value
      },
      set (newValue) {
        this.$emit('input', newValue)
      }
    },
    rePattern () {
      return new RegExp(this.pattern)
    },
    valid () {
      if (this.rePattern.test(this.model)) return ''
      return this.invalidMessage
    }
  }
}
</script>

<style scoped>
</style>
