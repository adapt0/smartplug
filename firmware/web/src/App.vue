<template>
  <div id="app">
    <div class="body">
      <Sidebar></Sidebar>
      <div class="content">
        <div class="blah">
          <router-view/>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'
import Sidebar from './components/Sidebar.vue'

export default {
  name: 'App',
  components: {
    Sidebar
  }
}
</script>

<style>
html, body, #app {
  height: 100%;
  width: 100%;
  overflow: hidden;
}

#app {
  display: flex;
  flex-direction: column;
}

#app > .body {
  flex: 1;
  display: flex;
}
#app > .body > .content {
  overflow: scroll;
  flex: 1;
  min-width: 0;
  padding: 1em;
}
</style>
