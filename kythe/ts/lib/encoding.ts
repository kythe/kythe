/*
 * Copyright 2015 Google Inc. All rights reserved.
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

import ts = require('typescript');
import tsu = require('./tsutilities');

/**
 * Maps from line number to UTF-16 code unit and from line number to UTF-8
 * code unit, both at the start of the line.
 *
 * Lines are a somewhat arbitrary choice for making these breaks (we could also
 * just scatter "enough" random offsets through the file).
 */
interface EncodingConversionCacheEntry {
  line16 : number[];
  line8 : number[];
}

let cache : ts.Map<EncodingConversionCacheEntry> = { };

function getCachedUtf8FileMap(file : ts.SourceFile)
    : EncodingConversionCacheEntry {
  let cached = cache[file.fileName];
  if (cached) {
    return cached;
  }
  let result : EncodingConversionCacheEntry =
      { line16: new Array(), line8: new Array() };
  let fileText = file.text;
  let utf8Offset = 0;
  let utf16Offset = 0;
  let warnedAboutBadEncoding = false;
  result.line16.push(utf16Offset);
  result.line8.push(utf8Offset);
  while (utf16Offset < fileText.length) {
    let codeUnit = fileText.charCodeAt(utf16Offset++);
    if (codeUnit <= 0x7F) {
      ++utf8Offset;
    } else if (codeUnit <= 0x7FF) {
      utf8Offset += 2;
    } else {
      utf8Offset += 3;
    }
    if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF
        && utf16Offset < fileText.length) {
      let succCodeUnit = fileText.charCodeAt(utf16Offset);
      if (succCodeUnit >= 0xDC00 && succCodeUnit <= 0xDFFF) {
        // This is a cromulent surrogate pair. Its encoding will always take
        // four UTF-8 code units (and we've already advanced our UTF-8 place
        // by three of these.) Drop the second UTF-16 code unit from
        // consideration. (None of the line break encodings we're interested
        // in have encodings that require surrogate pairs.)
        ++utf8Offset;
        ++utf16Offset;
      } else {
        // The second UTF-16 code unit was not the second part of a surrogate
        // pair. (This encoding is actually invalid.)
        if (!warnedAboutBadEncoding) {
          console.warn(file.fileName, "is encoded incorrectly.");
          warnedAboutBadEncoding = true;
        }
      }
    } else if (ts.isLineBreak(codeUnit)) {
      if (codeUnit === 0x0D
          && utf16Offset < fileText.length
          && fileText.charCodeAt(utf16Offset) === 0x0A) {
        // On systems that use \d\n for newlines, don't store twice as many
        // entries than we need.
        ++utf16Offset;
        ++utf8Offset;
      }
      result.line16.push(utf16Offset);
      result.line8.push(utf8Offset);
    }
  }
  return result;
}

/**
 * Given a position in some file, returns the equivalent offset in a UTF-8
 * encoding of that file. We assume that the file is properly encoded
 * (that is, there are no bad surrogate pairs).
 */
export function getUtf8PositionOfSourcePosition(
    file : ts.SourceFile, position : number) : number {
  let fileText = file.text;
  if (position < 0 || position >= fileText.length) {
    return position;
  }
  let utf8FileMap = getCachedUtf8FileMap(file);
  // Binary search to find the predecessor or equal offset to position.
  // (The cache contains an entry for 0.)
  let index = tsu.binarySearch(utf8FileMap.line16, position);
  if (index < 0) {
    index = ~index - 1;
  }
  let utf16Offset = utf8FileMap.line16[index];
  let utf8Offset = utf8FileMap.line8[index];
  while (utf16Offset < position) {
    let codeUnit = fileText.charCodeAt(utf16Offset++);
    if (codeUnit <= 0x7F) {
      ++utf8Offset;
    } else if (codeUnit <= 0x7FF) {
      utf8Offset += 2;
    } else if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF
               && utf16Offset < fileText.length) {
      let succCodeUnit = fileText.charCodeAt(utf16Offset);
      if (succCodeUnit >= 0xDC00 && succCodeUnit <= 0xDFFF) {
        ++utf16Offset;
        utf8Offset += 4;
      } else {
        // Bad encoding.
        utf8Offset += 3;
      }
    } else {
      utf8Offset += 3;
    }
  }
  return utf8Offset;
}
