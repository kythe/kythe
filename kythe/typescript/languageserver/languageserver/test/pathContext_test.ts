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

import {LocalPath, normalizeLSPath, PathContext} from '../src/pathContext';

describe('PathContext', () => {
  describe('ticket', () => {
    const context = new PathContext(
        '/', [{
          local: 'root/:root/:corpus/:path*',
          vname: {corpus: 'C/:corpus', path: 'P/:path', root: 'R/:root'}
        }]);

    it('should return an error if no patterns match', () => {
      const ticket = context.ticket('/foo/bar' as LocalPath);
      assert.instanceOf(ticket, Error);
    });

    it('should produce a valid ticket for matching paths', () => {
      const ticket =
          context.ticket('/root/myRoot/myCorpus/my/path' as LocalPath);

      assert.deepEqual(
          ticket, {path: 'P/my/path', corpus: 'C/myCorpus', root: 'R/myRoot'});
    });

  });

  describe('local', () => {
    const context = new PathContext(
        '/root', [{
          local: ':corpus/:path',
          vname: {corpus: ':corpus', path: ':path', root: 'myRoot'}
        }]);

    it('should require matching constant members', () => {
      const ticket = {corpus: 'myCorpus', path: 'myPath'};

      const path = context.local(ticket);
      assert.instanceOf(path, Error);
    });

    it('should prepend root', () => {
      const ticket = {corpus: 'myCorpus', path: 'myPath', root: 'myRoot'};

      const path = context.local(ticket);
      assert.strictEqual('/root/myCorpus/myPath' as LocalPath, path);
    });
  });

  describe('normalizeLSPath', () => {
    it('should strip git path extras', () => {
      const lsPath = 'git:/dir/README.md.git?path=/dir/README.md';
      const path = normalizeLSPath(lsPath);

      assert.strictEqual('/dir/README.md' as LocalPath, path);
    });

    it('should strip file protocol', () => {
      const path = normalizeLSPath('file:///root/README.md');

      assert.strictEqual('/root/README.md' as LocalPath, path);
    });
  });
});
