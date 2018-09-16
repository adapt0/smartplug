<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<template>
  <div class="update">
    <div class="header d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center">
      <h1 class="h2">Upgrade Settings</h1>
    </div>
    <div class="content">
      <h5>{{$store.state.Rpc.data.version}}</h5>
      <button v-on:click="beginUpdate" :disabled='updateInProgress'>Update firmware</button>

      <ul>
        <li>Multi-client feedback</li>
        <li>From GitHub?</li>
      </ul>

      <b-modal ref="modalUpdate" centered no-close-on-backdrop no-close-on-esc hide-header hide-footer>
        <div class="container-fluid">
          <div class="row py-3">
            <div class="col-sm">&nbsp;</div>
            <div class="col-md-auto text-center">
              <h4 class="mb-0">Update in progress...</h4>
            </div>
            <div class="col-sm text-right">
              <icon class="align-bottom" name="spinner" pulse />
            </div>
          </div>
        </div>
      </b-modal>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import { Modal } from 'bootstrap-vue';
import 'vue-awesome/icons/spinner';

@Component
export default class SettingsUpgrade extends Vue {
  public updateInProgress = false;

  public beginUpdate() {
    // https://stackoverflow.com/questions/13688814/uploading-a-file-with-a-single-button
    // https://github.com/pagekit/vue-resource/blob/master/docs/recipes.md#forms
    const fileButton = document.createElement('input');
    fileButton.type = 'file';
    fileButton.onchange = async () => {
      // sanity check file
      const file = fileButton.files && fileButton.files[0];
      if (!file) { return; }

      // build up + submit form
      const formData = new FormData();
      formData.append('file', file);
      const eModalUpdate = (this.$refs.modalUpdate instanceof Vue) ? this.$refs.modalUpdate as Modal : undefined;
      try {
        if (eModalUpdate) { eModalUpdate.show(); }
        this.updateInProgress = true;

        // post
        const promisePost = this.$http.post('/api/v1/update', formData);

        // wait for reconnect
        const promiseReconnect = new Promise((resolve) => {
          let unwatch: (() => void) | undefined;
          let timerId: NodeJS.Timer | undefined;
          const done = () => {
            if (timerId) { clearTimeout(timerId); timerId = undefined; }
            if (unwatch) { unwatch(); unwatch = undefined; }
            resolve();
          };

          unwatch = this.$store.watch(
            (state) => this.$store.state.Rpc.connected,
            (newValue, oldValue) => {
              if (!oldValue && newValue) { done(); }
            },
          );
          timerId = setTimeout(done, 30000);
        });

        await Promise.race([
          promisePost, promiseReconnect,
        ]);
      } catch (e) {
        alert(e.statusText);
      } finally {
        if (eModalUpdate) { eModalUpdate.hide(); }
        this.updateInProgress = false;
      }
    };

    document.body.appendChild(fileButton);
    fileButton.click();
    fileButton.remove();
  }
}
</script>
