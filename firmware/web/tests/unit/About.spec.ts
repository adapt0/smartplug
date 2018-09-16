/** @file
 * Test About.vue
 *
 * \copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
 * Licensed under the MIT License. Refer to LICENSE file in the project root.
 */
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
                version: 'VERSION',
              },
            },
          },
        },
      },
    });
    expect(wrapper.text()).to.include('SmartPlug VERSION');
  });
});
