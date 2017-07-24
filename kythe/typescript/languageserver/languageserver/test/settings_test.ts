/*
 * Copyright 2017 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {assert} from 'chai';
import {resolve} from 'path';

import {findRoot, parseSettings} from '../src/settings';

describe('Settings', () => {
  describe('parseSettings', () => {
    const badSettings = [
      ['empty object', {}], ['empty mappings', {mappings: []}],
      ['missing vname', {mappings: [{local: ':path'}]}],
      ['missing path', {mappings: [{local: ':path', vname: {}}]}],
      ['missing local', {mappings: [{vname: {path: ':path'}}]}],
      [
        'empty xrefs',
        {mappings: [{local: ':path', vname: {path: ':path'}}], xrefs: {}}
      ]
    ];

    for (const [description, settings] of badSettings) {
      it(`should reject ${description}`, () => {
        assert.instanceOf(
            parseSettings(settings), Error, JSON.stringify(settings));
      });
    }


    const goodSettings = [
      ['omitted xrefs', {mappings: [{local: ':path', vname: {path: ':path'}}]}],
      [
        'fully specified', {
          mappings: [{local: ':path', vname: {path: ':path'}}],
          xrefs: {host: 'localhost', port: 8080}
        }
      ],
    ];

    for (const [description, settings] of goodSettings) {
      it(`should accept ${description}`, () => {
        assert.notInstanceOf(
            parseSettings(settings), Error, JSON.stringify(settings));
      });
    }
  });

  describe('findRoot', () => {
    const settingsRoot =
        resolve(__dirname, '..', '..', 'test', 'settings_test');
    const goodDirs = ['', 'sub'];

    for (const dir of goodDirs) {
      it(`should find the bad settings file in settings_test/${dir}`, () => {
        const root = findRoot(resolve(settingsRoot, dir));
        assert.deepEqual(root, settingsRoot);
      });
    }
  });
});
