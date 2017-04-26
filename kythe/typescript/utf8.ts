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

/**
 * This module defines an "OffsetTable", which maps UTF-16 offsets (used by
 * TypeScript) to byte offsets (used by Kythe).
 *
 * This module assumes inputs are already valid UTF-8.
 */

/**
 * Scanner scans a UTF-8 Buffer across Unicode codepoints one by one.  It
 * exposes a public offset member to see the current (byte) offset.
 */
class Scanner {
  constructor(private buf: Buffer, public ofs = 0) {}

  /** Scans forward one codepoint. */
  scan(): void {
    let byte = this.buf[this.ofs++];

    if ((byte & 0b10000000) === 0) {
      // Common case: ASCII.
      return;
    }

    if ((byte & 0b11100000) == 0b11000000) {
      this.ofs += 1;
    } else if ((byte & 0b11110000) == 0b11100000) {
      this.ofs += 2;
    } else if ((byte & 0b11111000) == 0b11110000) {
      this.ofs += 3;
    } else {
      throw new Error(`unhandled UTF-8 byte 0x${byte.toString(16)}`)
    }
  }
}

/**
 * OffsetTable caches a UTF-16 -> UTF-8 offset mapping.
 */
export class OffsetTable {
  /**
   * Holds [utf16 offset, byte offset] pairs, with each entry at least spanSize
   * after the one before.
   *
   * Hypothetically if spanSize was 1, then the table would hold
   *   [0, 0]
   *   [1, byte offset of first character]
   *   [2, byte offset of second character]
   * and so on.
   *
   * When spanSize is greater than 1, we skip intermediate entries,
   * so the first entry after zero is [spanSize, ...] and the second is
   * [spanSize*2, ...].
   *
   * To look up the byte offset of an input UTF-16 offset, we find the first
   * span occurring before the queried offset (which we can compute using simple
   * math using spanSize) and then repeat the scan forwards to find the offset
   * within the span.
   *
   * A larger spanSize saves memory (fewer entries in the table) at the cost
   * of more CPU (need to do more scanning to find an offset).  In practice
   * it doesn't really matter that much because our input files are pretty
   * small, but the table is at least nice to prevent an O(1) scan from the
   * beginning of the file for every requested offset.
   */
  offsets: Array<[number, number]>;

  constructor(private buf: Buffer, private spanSize: number = 128) {
    this.build(buf);
  }

  private build(bytes: Buffer) {
    this.offsets = [];
    let scanner = new Scanner(bytes);
    let ofs = 0;
    let lastEntry = 0;
    this.offsets.push([ofs, scanner.ofs]);
    while (scanner.ofs < bytes.length) {
      scanner.scan();
      ofs++;
      if (ofs - lastEntry >= this.spanSize) {
        this.offsets.push([ofs, scanner.ofs]);
        lastEntry += this.spanSize;
      }
    }
  }

  lookup(findOfs: number): number {
    let [u16, byte] = this.offsets[Math.floor(findOfs / this.spanSize)];
    let scanner = new Scanner(this.buf, byte);
    while (u16 < findOfs) {
      scanner.scan();
      u16++;
    }
    return scanner.ofs;
  }
}
