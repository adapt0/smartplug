<template>
  <div class='toggle btn btn-default' v-bind:class='{ off: !value, disabled }' style='width: 70px; height: 38px;' v-on:click='toggle'>
    <input type='checkbox' checked=''>
    <div class='toggle-group'>
      <label class='btn btn-primary toggle-on'>{{onValue}}</label>
      <label class='btn btn-secondary active toggle-off'>{{offValue}}</label>
      <span class='toggle-handle btn btn-light'></span>
    </div>
  </div>
</template>

<script>
export default {
  props: {
    value: {
      type: Boolean,
      default: false
    },
    disabled: {
      type: Boolean,
      default: false
    },
    onValue: {
      type: String,
      default: 'On'
    },
    offValue: {
      type: String,
      default: 'Off'
    }
  },
  methods: {
    toggle () {
      if (!this.disabled) {
        this.$emit('input', !this.value)
      }
    }
  }
}
</script>

<style scoped>
/*! ========================================================================
 * Bootstrap Toggle: bootstrap2-toggle.css v2.2.0
 * http://www.bootstraptoggle.com
 * ========================================================================
 * Copyright 2014 Min Hur, The New York Times Company
 * Licensed under MIT
 * ======================================================================== */

label.checkbox .toggle,
label.checkbox.inline .toggle {
  margin-left: -20px;
  margin-right: 5px;
}
.toggle {
  min-width: 40px;
  height: 20px;
  position: relative;
  overflow: hidden;
}
.toggle input[type="checkbox"] {
  display: none;
}
.toggle-group {
  position: absolute;
  width: 200%;
  top: 0;
  bottom: 0;
  left: 0;
  transition: left 0.35s;
  -webkit-transition: left 0.35s;
  -moz-user-select: none;
  -webkit-user-select: none;
}
.toggle.off .toggle-group {
  left: -100%;
}
.toggle-on {
  position: absolute;
  top: 0;
  bottom: 0;
  left: 0;
  right: 50%;
  margin: 0;
  border: 0;
  border-radius: 0;
}
.toggle-off {
  position: absolute;
  top: 0;
  bottom: 0;
  left: 50%;
  right: 0;
  margin: 0;
  border: 0;
  border-radius: 0;
}
.toggle-handle {
  position: relative;
  margin: 0 auto;
  padding-top: 0px;
  padding-bottom: 0px;
  height: 100%;
  width: 0px;
  border-width: 0 1px;
}
.toggle-handle.btn-mini {
  top: -1px;
}
.toggle.btn { min-width: 30px; }
.toggle-on.btn { padding-right: 24px; }
.toggle-off.btn { padding-left: 24px; }

.toggle.btn-large { min-width: 40px; }
.toggle-on.btn-large { padding-right: 35px; }
.toggle-off.btn-large { padding-left: 35px; }

.toggle.btn-small { min-width: 25px; }
.toggle-on.btn-small { padding-right: 20px; }
.toggle-off.btn-small { padding-left: 20px; }

.toggle.btn-mini { min-width: 20px; }
.toggle-on.btn-mini { padding-right: 12px; }
.toggle-off.btn-mini { padding-left: 12px; }

/* https://github.com/minhur/bootstrap-toggle/issues/186 */
/* Bootstrap Toggle v2.2.2 corrections for Bootsrtap 4*/
.toggle-off {
    box-shadow: inset 0 3px 5px rgba(0, 0, 0, .125);
}
.toggle.btn {
    border-color: rgba(0, 0, 0, .25);
}
</style>
