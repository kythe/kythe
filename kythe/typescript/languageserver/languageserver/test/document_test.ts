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
import {Position, Range} from 'vscode-languageserver';

import {Document} from '../src/document';
import {KytheTicketString} from '../src/pathContext';

describe('Document', () => {
  describe('xrefs', () => {

    it('should return the smallest matching Range', () => {
      const text = 'abcdefg';
      const document = new Document(
          [
            {
              range: Range.create(Position.create(0, 0), Position.create(0, 5)),
              target: 'longest' as KytheTicketString
            },
            {
              range: Range.create(Position.create(0, 2), Position.create(0, 3)),
              target: 'shortest' as KytheTicketString
            },
            {
              range: Range.create(Position.create(0, 2), Position.create(0, 4)),
              target: 'middle' as KytheTicketString
            }
          ],
          text);

      assert.equal('shortest', document.xrefs(Position.create(0, 2)) as String);
    });

    it('should map references to dirty state', () => {
      const text = 'hi there';
      const document = new Document(
          [{
            range: Range.create(Position.create(0, 0), Position.create(0, 2)),
            target: 'hi' as KytheTicketString
          }],
          text);

      document.updateDirtyState('  hi there');

      assert.equal('hi', document.xrefs(Position.create(0, 3)) as String);
    });
  });
});
