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

import * as diff from 'diff-match-patch';
import {Position, Range} from 'vscode-languageserver';

import {KytheTicketString} from './pathContext';

export interface RefResolution {
  range: Range;
  target: KytheTicketString;
  newRange?: Range;
}

// Provides a numeric value representing the size of the range. This value is
// weighted to exaggerate line differences
function size(range: Range): number {
  return (range.end.line - range.start.line) * 1000 +
      (range.end.character - range.start.character);
}

export class Document {
  // Tracks whether refs need to be regenerated because the dirty state has
  // changed
  private staleRefs = true;

  constructor(
      private refs: RefResolution[], private sourceText: string,
      private dirtyText: string = sourceText) {
    // Sort by size so we get quick resolution on lookups
    refs.sort(
        (a, b) => b === undefined ? 1 :
                                    a.range.start.line - b.range.start.line ||
                a.range.start.character - b.range.start.character);
  }

  xrefs(loc: Position): KytheTicketString|undefined {
    if (this.staleRefs) {
      this.generateDirtyRefs();
    }

    return this.refs.filter(r => rangeContains(range(r), loc))
        .sort((a, b) => size(range(a)) - size(range(b)))
        .map(r => r.target)[0];
  }

  // Takes a range from the original file in Kythe and returns that range mapped
  // into the dirty state
  dirtyRange(r: Range): Range|Error {
    if (this.staleRefs) {
      this.generateDirtyRefs();
    }

    const ref = this.refs.find(
        ref => ref.range.start.character === r.start.character &&
            ref.range.start.line === r.start.line &&
            ref.range.end.line === r.end.line &&
            ref.range.end.character === r.end.character);
    if (ref === undefined) {
      return new Error(`Unable to map range: ${r}`);
    }
    return range(ref);
  }

  updateDirtyState(text: string) {
    this.dirtyText = text;
    this.staleRefs = true;
  }

  // Diffs the current dirtyText and the sourceText, generating the shifted
  // ranges for all the refs in the file
  private generateDirtyRefs() {
    const differ = new diff.diff_match_patch();
    const diffs = differ.diff_main(this.sourceText, this.dirtyText);
    differ.diff_cleanupSemantic(diffs);
    const oldPosition: Position = {line: 0, character: 0};

    const newPosition: Position = {line: 0, character: 0};

    let refIndex = 0;

    for (const d of diffs) {
      // If we're past the ref with respect to the old file, shift refIndex
      while (refIndex < this.refs.length &&
             (this.refs[refIndex].range.start.line < oldPosition.line ||
              (this.refs[refIndex].range.start.line === oldPosition.line &&
               this.refs[refIndex].range.start.character <
                   oldPosition.character))) {
        refIndex++;
      }

      if (this.refs.length <= refIndex) {
        break;
      }

      // split the diff on lines
      const ldiff = d[1].split('\n');
      const newLine = ldiff.length > 1;
      const charOffset = ldiff[ldiff.length - 1].length;
      const diffLineLength = ldiff.length - 1;

      switch (d[0]) {
        case diff.DIFF_EQUAL:
          // The end of the range in the old file which has remained the same
          const oldEndPosition = {
            line: oldPosition.line + diffLineLength,
            character: newLine ? charOffset : oldPosition.character + charOffset
          };

          const diffRange = {start: oldPosition, end: oldEndPosition};

          // Loop through until we find a ref that starts after the diff
          for (let i = refIndex; i < this.refs.length &&
               rangeContains(diffRange, this.refs[i].range.start);
               i++) {
            // If this ref extends past the end of the diff it can't be used
            if (!rangeContains(diffRange, this.refs[i].range.end)) {
              continue;
            }

            const startLine = newPosition.line +
                (this.refs[i].range.start.line - oldPosition.line);
            const onDifferentLine = startLine !== newPosition.line;
            const refLineLength =
                this.refs[i].range.end.line - this.refs[i].range.start.line;

            // If the ref starts on a different line, newPosition.character
            // won't affect the starting character of the shifted ref
            const startChar = onDifferentLine ?
                this.refs[i].range.start.character :
                newPosition.character +
                    (this.refs[i].range.start.character -
                     oldPosition.character);

            const refCharLength =
                (this.refs[i].range.end.character -
                 this.refs[i].range.start.character);

            this.refs[i].newRange = {
              start: {line: startLine, character: startChar},
              end: {
                line: startLine + refLineLength,
                // If the ref is a single line, just add the length to the
                // startChar
                character: refLineLength > 0 ?
                    this.refs[i].range.end.character :
                    startChar + refCharLength
              }
            };
          }

        // If anything was deleted, we've made progress through the old file
        // but haven't moved our new position
        case diff.DIFF_DELETE:
          if (newLine) {
            oldPosition.line += diffLineLength;
            oldPosition.character = charOffset;
          } else {
            oldPosition.character += charOffset;
          }
          if (d[0] === diff.DIFF_DELETE) {
            break;
          }

        // If anything was inserted, we've made progress through the new file
        // but haven't moved our old position
        case diff.DIFF_INSERT:
          if (newLine) {
            newPosition.line += diffLineLength;
            newPosition.character = charOffset;
          } else {
            newPosition.character += charOffset;
          }
          break;


        default:
          break;
      }
    }

    this.staleRefs = false;
  }
}


function range(ref: RefResolution): Range {
  return ref.newRange || ref.range;
}

function rangeContains(range: Range, pos: Position) {
  // Some editors (i.e. vscode) use the character after an identifier for
  // positions so we treat ranges as inclusive on both ends
  return (range.start.line < pos.line ||
          (range.start.line === pos.line &&
           range.start.character <= pos.character)) &&
      (range.end.line > pos.line ||
       (range.end.line === pos.line && range.end.character >= pos.character));
}
