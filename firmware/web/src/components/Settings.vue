<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="settings">
    <b-tabs card no-fade v-model="tabIndex">
      <b-tab title="General" active>
        <ul>
          <li>Button state persistence</li>
          <li>Scheduling</li>
          <li>Automated power cycling tests</li>
          <li>Web log/console?</li>
          <li>REST</li>
          <li>Home automation interfacing</li>
        </ul>
      </b-tab>
      <b-tab title="Network">
        <NetworkSettings />
      </b-tab>
      <b-tab title="Security" >
        <ul>
          <li>Disable local button</li>
          <li>Disable remote button</li>
          <li>Security</li>
        </ul>
      </b-tab>
      <b-tab title="Upgrade" >
        <UpgradeSettings />
      </b-tab>
    </b-tabs>
  </div>
</template>

<script>
import NetworkSettings from '@/components/settings/Network'
import UpgradeSettings from '@/components/settings/Upgrade'

export default {
  name: 'Settings',
  components: {
    NetworkSettings,
    UpgradeSettings
  },
  data () {
    return {
      tabIndex: 0
    }
  }
}
</script>

<style scoped>
form {
  max-width: 640px;
}
</style>
