<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="network">
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
                    v-model="form.apSsid" id="inputSsid"
                    pattern=".+$"
                    invalidMessage='Please enter a valid SSID'
                    required />
          <FormInput label="Password:" placeholder="" class="mb-1"
                    v-model="form.apPassword" id="inputPassword" />
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
            <FormInput label="Default Gateway:"
                      :value="$store.state.Rpc.data.sys.net.cur.ipv4Gateway" readonly />
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
            <FormInput placeholder="Enter a default gateway" label="Default Gateway:" class="mb-1"
                      v-model="form.ipv4Gateway" id="inputIpv4Gateway"
                      pattern="((^|\.)((25[0-5])|(2[0-4]\d)|(1\d\d)|([1-9]?\d))){4}$"
                      invalidMessage='Please enter an IP address' />
          </element>
        </b-form-group>
        <b-alert variant="danger" :show="formAlert">{{formAlert}}</b-alert>
        <b-button type="submit" variant="primary">Submit</b-button>
      </fieldset>
    </b-form>
  </div>
</template>

<script>
import FormInput from '@/components/shared/FormInput'

export default {
  components: {
    FormInput
  },
  data () {
    return {
      dhcpOptions: [
        { value: true, text: 'Using DHCP' },
        { value: false, text: 'Manually' }
      ],
      form: {
        dhcp: false,
        hostname: this.$store.state.Rpc.data.sys.net.hostname,
        apSsid: this.$store.state.Rpc.data.sys.net.ssid,
        apPassword: '',
        ipv4Address: this.$store.state.Rpc.data.sys.net.ipv4Address,
        ipv4Subnet: this.$store.state.Rpc.data.sys.net.ipv4Subnet,
        ipv4Gateway: this.$store.state.Rpc.data.sys.net.ipv4Gateway
      },
      formAlert: false,
      formDisabled: false
    }
  },
  methods: {
    /// submit network settings
    async onNetworkSubmit (evt) {
      evt.preventDefault()
      try {
        this.formDisabled = true
        const res = this.$store.dispatch('Rpc/network', this.form)
        console.log('res', res)
      } catch (e) {
        this.formAlert = `Error: ${e && e.message}`
      } finally {
        this.formDisabled = false
      }
    }
  }
}
</script>
