/** @file
 * Object merging
 * https://stackoverflow.com/questions/27936772/how-to-deep-merge-instead-of-shallow-merge
 *
 * \copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
 * Licensed under the MIT License. Refer to LICENSE file in the project root.
 */
import Vue from 'vue';

/**
 * Simple object check.
 * @param item
 * @returns {boolean}
 */
export function isObject(item: any) {
  return (item && typeof item === 'object' && !Array.isArray(item));
}

/**
 * Deep merge two (or more) objects
 * @param target
 * @param ...sources
 */
export function mergeDeep(target: any, ...sources: any[]): any {
  if (!sources.length) {
    return target;
  }
  const source = sources.shift();

  if (isObject(target) && isObject(source)) {
    for (const key in source) {
      if (isObject(source[key])) {
        if (!target[key]) {
          Vue.set(target, key, {});
        }
        mergeDeep(target[key], source[key]);
      } else {
        Vue.set(target, key, source[key]);
      }
    }
  }

  return mergeDeep(target, ...sources);
}
