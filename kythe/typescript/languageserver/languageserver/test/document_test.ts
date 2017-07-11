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
      const document = new Document([
        {
          range: Range.create(Position.create(1, 1), Position.create(1, 5)),
          target: 'longest' as String as KytheTicketString
        },
        {
          range: Range.create(Position.create(1, 2), Position.create(1, 3)),
          target: 'shortest' as String as KytheTicketString
        },
        {
          range: Range.create(Position.create(1, 2), Position.create(1, 4)),
          target: 'middle' as String as KytheTicketString
        }
      ]);

      assert.equal('shortest', document.xrefs(Position.create(1, 2)) as String);
    });

  });
});
