/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

import {OffsetTable} from './utf8';

describe('utf8 create offset table', () => {
  it('should handle 1-byte character encoding', () => {
    const buf = Buffer.from('123');
    const table = new OffsetTable(buf, 1);
    expect(table.buf.length).toEqual(3);
    expect(table.offsets).toEqual([[0, 0], [1, 1], [2, 2], [3, 3]]);
  });

  it('should handle 3-byte character encoding', () => {
    const buf = Buffer.from('12•3');
    const table = new OffsetTable(buf, 1);
    // Number of bytes = 1 + 1 + 3 + 1 = 6
    expect(table.buf.length).toEqual(6);
    expect(table.offsets).toEqual([[0, 0], [1, 1], [2, 2], [3, 5], [4, 6]]);
  });

  it('should handle 4-byte character encoding', () => {
    const buf = Buffer.from('12🐶3');
    const table = new OffsetTable(buf, 1);
    // Number of bytes = 1 + 1 + 4 + 1 = 7
    expect(table.buf.length).toEqual(7);
    expect(table.offsets).toEqual([
      [0, 0],
      [1, 1],
      [2, 2],
      // utf16 offset 3 is skipped because it's within a surrogate pair.
      [4, 6],
      [5, 7],
    ]);
  });

  it('should handle mix of 3-byte and 4-byte character encoding', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 1);
    // Number of bytes = 4 + 3 + 4 + 3 = 14
    expect(table.buf.length).toEqual(14);
    expect(table.offsets).toEqual([[0, 0], [2, 4], [3, 7], [5, 11], [6, 14]]);
  });

  it('should work when span size is greater than 1', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 2);
    // Number of bytes = 4 + 3 + 4 + 3 = 14
    expect(table.buf.length).toEqual(14);
    expect(table.offsets).toEqual([
      [0, 0],
      [2, 4],
      // utf16 offset 4 is skipped because it's within a surrogate pair.
      // Backoff to use offset 5.
      [5, 11],
      [6, 14],
    ]);
  });

  it('should work when span size is greater than Buffer size', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 32);
    // Number of bytes = 4 + 3 + 4 + 3 = 14
    expect(table.buf.length).toEqual(14);
    expect(table.offsets).toEqual([[0, 0]]);
  });
});

describe('lookupUtf8', () => {
  it('should throw an error at invalid lookup positions', () => {
    const buf = Buffer.from('🐶');
    const table = new OffsetTable(buf, 1);
    expect(() => table.lookupUtf8(0)).not.toThrow();
    // offset 1 is within a surrogate pair so it's invalid.
    expect(() => table.lookupUtf8(1))
        .toThrowError('The lookup offset is invalid');
  });

  it('should find the offsets when span size is greater than 1', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 32);
    expect(table.lookupUtf8(0)).toEqual(0);
    expect(table.lookupUtf8(2)).toEqual(4);
    expect(table.lookupUtf8(3)).toEqual(7);
    expect(table.lookupUtf8(5)).toEqual(11);
    expect(table.lookupUtf8(6)).toEqual(14);
  });

  it('should find the offsets when there are multiple spans', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 2);
    expect(table.lookupUtf8(0)).toEqual(0);
    expect(table.lookupUtf8(2)).toEqual(4);
    expect(table.lookupUtf8(3)).toEqual(7);
    expect(table.lookupUtf8(5)).toEqual(11);
    expect(table.lookupUtf8(6)).toEqual(14);
  });
});

describe('lookupUtf16', () => {
  it('should throw an error at invalid lookup positions', () => {
    const buf = Buffer.from('🐶');
    const table = new OffsetTable(buf, 1);
    expect(() => table.lookupUtf16(0)).not.toThrow();
    // offset 1 is within a surrogate pair so it's invalid.
    expect(() => table.lookupUtf16(1))
        .toThrowError('The lookup offset is invalid');
  });

  it('should find the offsets when span size is greater than 1', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 32);
    expect(table.lookupUtf16(0)).toEqual(0);
    expect(table.lookupUtf16(4)).toEqual(2);
    expect(table.lookupUtf16(7)).toEqual(3);
    expect(table.lookupUtf16(11)).toEqual(5);
    expect(table.lookupUtf16(14)).toEqual(6);
  });

  it('should find the offsets when there are multiple spans', () => {
    const buf = Buffer.from('🐶•🐶•');
    const table = new OffsetTable(buf, 2);
    expect(table.lookupUtf16(0)).toEqual(0);
    expect(table.lookupUtf16(4)).toEqual(2);
    expect(table.lookupUtf16(7)).toEqual(3);
    expect(table.lookupUtf16(11)).toEqual(5);
    expect(table.lookupUtf16(14)).toEqual(6);
  });
});
