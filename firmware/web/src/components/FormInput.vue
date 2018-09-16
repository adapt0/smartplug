<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <b-form-group :label="label" :label-for="id" :invalid-feedback="(required || model && model.length) ? valid : ''" :state="false" :required="required" horizontal>
    <b-form-input v-model="model" :id="id" :placeholder="placeholder" :readonly="readonly"/>
  </b-form-group>
</template>

<script lang="ts">
import { Component, Prop, Vue } from 'vue-property-decorator';

@Component
export default class FormInput extends Vue {
  @Prop(String) public id!: string;
  @Prop(String) public label!: string;
  @Prop(String) public value!: string;
  @Prop(String) public pattern!: string;
  @Prop(String) public invalidMessage!: string;
  @Prop(String) public placeholder!: string;
  @Prop({ type: Boolean, default: false }) public readonly!: boolean;
  @Prop({ type: Boolean, default: false }) public required!: boolean;

  get model() {
    return this.value;
  }
  set model(newValue) {
    this.$emit('input', newValue);
  }
  get rePattern() {
    return new RegExp(this.pattern);
  }
  get valid() {
    if (this.rePattern.test(this.model)) { return ''; }
    return this.invalidMessage;
  }
}
</script>

<style lang="scss" scoped>
</style>
