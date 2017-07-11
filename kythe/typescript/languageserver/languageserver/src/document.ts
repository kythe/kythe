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

import {Position, Range} from 'vscode-languageserver';
import {KytheTicketString} from './pathContext';

export interface RefResolution {
  range: Range;
  target: KytheTicketString;
}

// Provides a numeric value representing the size of the range. This value is
// weighted to exaggerate line differences
function size(range: Range): number {
  return (range.end.line - range.start.line) * 1000 +
      (range.end.character - range.start.character);
}

export class Document {
  constructor(private refs: RefResolution[]) {
    // Sort by size so we get quick resolution on lookups
    refs.sort((a, b) => size(a.range) - size(b.range));
  }

  xrefs(loc: Position): KytheTicketString|undefined {
    return this.refs.filter(r => rangeContains(r.range, loc))
        .map(r => r.target)[0];
  }
}

function rangeContains(range: Range, pos: Position) {
  return ((range.start.line === pos.line &&
           range.start.character <= pos.character)) &&
      ((range.end.line === pos.line && range.end.character > pos.character));
}
