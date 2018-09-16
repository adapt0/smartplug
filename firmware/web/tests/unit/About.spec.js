import { expect } from 'chai';
import { shallowMount } from '@vue/test-utils';
import About from '@/views/About.vue';

describe('About.vue', () => {
  it('renders', () => {
    const wrapper = shallowMount(About, {
      mocks: {
        $store: {
          state: {
            Rpc: {
              data: {
                version: 'VERSION'
              }
            }
          }
        }
      }
    });
    expect(wrapper.text()).to.include('SmartPlug VERSION');
  });
});
