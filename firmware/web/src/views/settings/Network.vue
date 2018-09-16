<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="network">
    <div class="header d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center">
      <h1 class="h2">Network Settings</h1>
    </div>
    <div class="content">
      <b-form @submit="onNetworkSubmit">
        <fieldset :disabled="formDisabled">
          <b-form-group label="Network" label-size="lg" class="mb-0">
            <FormInput label="Hostname:" placeholder="Enter hostname" class="mb-1"
                      v-model="form.hostname" id="inputHostname"
                      pattern="\w+$"
                      invalidMessage='Please enter a valid hostname'
                      required />
          </b-form-group>
          <b-form-group label="Wireless AP" label-size="lg" class="mb-0">
            <FormInput label="Network SSID:" placeholder="Enter SSID" class="mb-1"
                      v-model="form.ssid" id="inputSsid"
                      pattern=".+$"
                      invalidMessage='Please enter a valid SSID'
                      required />
            <FormInput label="Password:" placeholder="" class="mb-1"
                      v-model="form.password" id="inputPassword" />
          </b-form-group>
          <b-form-group label="TCP/IP" label-size="lg" class="mb-0">
            <b-form-group label="Configure IPv4:" label-for="inputDhcp" horizontal>
              <b-form-select id="inputDhcp" v-model="form.dhcp" :options="dhcpOptions" />
            </b-form-group>
            <element v-if="form.dhcp">
              <FormInput label="IPv4 Address:" class="mb-1"
                        :value="$store.state.Rpc.data.sys.net.cur.ipv4Address" readonly />
              <FormInput label="Subnet Mask:" class="mb-1"
                        :value="$store.state.Rpc.data.sys.net.cur.ipv4Subnet" readonly />
              <FormInput label="Default Gateway:" class="mb-3"
                        :value="$store.state.Rpc.data.sys.net.cur.ipv4Gateway" readonly />
              <FormInput label="Primary DNS:" class="mb-1"
                        :value="$store.state.Rpc.data.sys.net.cur.ipv4Dns1" readonly />
              <FormInput label="Secondary DNS:"
                        :value="$store.state.Rpc.data.sys.net.cur.ipv4Dns2" readonly />
            </element>
            <element v-else>
              <FormInput label="IPv4 Address:" placeholder="Enter IPv4 address" class="mb-1"
                        v-model="form.ipv4Address" id="inputIpv4Address"
                        pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                        invalidMessage='Please enter an IP address'
                        required />
              <FormInput label="Subnet Mask:" placeholder="Enter subnet mask" class="mb-1"
                        v-model="form.ipv4Subnet" id="inputIpv4Subnet"
                        pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                        invalidMessage='Please enter a subnet mask'
                        required />
              <FormInput placeholder="Enter a default gateway" label="Default Gateway:" class="mb-3"
                        v-model="form.ipv4Gateway" id="inputIpv4Gateway"
                        pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                        invalidMessage='Please enter an IP address' />
              <FormInput placeholder="Enter a primary DNS" label="Primary DNS:" class="mb-1"
                        v-model="form.ipv4Dns1" id="inputIpv4Dns1"
                        pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                        invalidMessage='Please enter a DNS address' />
              <FormInput placeholder="Enter a secondary DNS" label="Secondary DNS:" class="mb-1"
                        v-model="form.ipv4Dns2" id="inputIpv4Dns2"
                        pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                        invalidMessage='Please enter a DNS address' />
            </element>
          </b-form-group>
          <b-alert variant="danger" :show="false !== formAlert">{{formAlert}}</b-alert>
          <b-button type="submit" variant="primary" class="mt-2">Submit</b-button>
        </fieldset>
      </b-form>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue, Watch } from 'vue-property-decorator';
import FormInput from '@/components/FormInput.vue';

@Component({ components: { FormInput } })
export default class SettingsNetwork extends Vue {

  get rpcConnected() {
    return this.$store.state.Rpc.connected;
  }
  public dhcpOptions = [
    { value: true, text: 'Using DHCP' },
    { value: false, text: 'Manually' },
  ];

  public form: any = {};
  public formAlert: string | false = false;
  public formDisabled = false;

  public mounted() {
    this.form = this.fillForm_();
  }

  /// submit network settings
  public async onNetworkSubmit(evt: Event) {
    evt.preventDefault();
    try {
      this.formDisabled = true;
      const res = await this.$store.dispatch('Rpc/network', this.form);
      if (!res) { throw new Error('Failed to apply network settings'); }
      this.formAlert = false;
    } catch (e) {
      this.formAlert = `Error: ${e && e.message}`;
    } finally {
      this.formDisabled = false;
    }
  }

  @Watch('rpcConnected')
  public onRpcConnected(connected: boolean) {
    // pick up settings on connection
    if (connected) { Object.assign(this.form, this.fillForm_()); }
  }

  private fillForm_() {
    const net = this.$store.state.Rpc.data.sys.net;
    return {
      dhcp: net.dhcp,
      hostname: net.hostname,
      ssid: net.ssid,
      password: '',
      ipv4Address: net.ipv4Address,
      ipv4Subnet: net.ipv4Subnet,
      ipv4Gateway: net.ipv4Gateway,
      ipv4Dns1: net.ipv4Dns1,
      ipv4Dns2: net.ipv4Dns2,
    };
  }
}
</script>

<style lang="scss" scoped>
form {
  max-width: 640px;
}
</style>
