<template>
  <b-nav class="sidebar" vertical>
    <b-nav-text><router-link to="/"><h5>SmartPlug</h5></router-link></b-nav-text>
    <b-nav-item to="home"><icon name="home"/>Home</b-nav-item>
    <b-nav-item to="settings"><icon name="cog"/>Settings</b-nav-item>
    <b-nav-item to="about"><icon name="info-circle"/>About</b-nav-item>
  </b-nav>
</template>

<script>
import 'vue-awesome/icons/cog'
import 'vue-awesome/icons/home'
import 'vue-awesome/icons/info-circle'

export default {
  name: 'Sidebar'
}
</script>

<style>
.sidebar {
  box-shadow: inset -1px 0 0 rgba(0, 0, 0, .1);
}

.sidebar .navbar-text {
  text-align: center;
}
.sidebar .navbar-text h5,
.sidebar .navbar-text a {
  color: #000;
  text-decoration: none;
  margin: 0;
}

.sidebar .nav-link {
  font-weight: 500;
  color: #333;
  display: flex;
  align-items: center;
  padding-right: 2em;
}

.sidebar .nav-link .fa-icon {
  margin-right: 8px;
}

.sidebar .nav-link .feather {
  margin-right: 4px;
  color: #999;
}

.sidebar .nav-link.disabled {
  color: #aaa;
}

.sidebar .nav-link.active {
  color: #007bff;
  background-color: rgba(0, 0, 0, 0.05);
}

.sidebar .nav-link:hover .feather,
.sidebar .nav-link.active .feather {
  color: inherit;
}
</style>
