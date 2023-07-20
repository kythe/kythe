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

#ifndef KYTHE_CXX_COMMON_UTF8_LINE_INDEX_H_
#define KYTHE_CXX_COMMON_UTF8_LINE_INDEX_H_

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace kythe {

// Returns whether a UTF-8 byte is a continuation byte, i.e., a byte other
// than the first byte of the encoding of a character.
bool IsUTF8ContinuationByte(int byte);

// Returns whether a UTF-8 byte from the |content| is the end of a line,
// i.e., a '\n', or a '\r' not immediately followed by a '\n'.
bool IsUTF8EndOfLineByte(int byte_offset, std::string_view content);

// Describes the position of a character in a file, in an encoding-independent
// way.  By encoding-independent, we mean that if the file were re-encoded in
// a different way (e.g., UCS-32 instead of UTF-8), the CharacterPosition
// would be unchanged.
struct CharacterPosition {
  // n-th character in file, 0-based, or -1 if this position is invalid.
  int character_number;

  // 1-based line number, or or -1 if this position is invalid.
  int line_number;

  // 0-based column number, or or -1 if this position is invalid.
  int column_number;

  // Checks whether this position is valid.  Invalid positions should
  // only arise either if this character position has never been set,
  // or if it was computed based on an invalid position.
  bool is_valid() const { return character_number != -1; }

  // By default, creates an invalid CharacterPosition.
  // Postcondition: !this->is_valid()
  CharacterPosition()
      : character_number(-1), line_number(-1), column_number(-1) {}
};

// Writes a debug representation of a CharacterPosition to an ostream.
std::ostream& operator<<(std::ostream& dest, const CharacterPosition& position);

// For a given text file, maps between byte offsets and CharacterPositions
// (character number, line number, column number).
//
// A line is terminated by LF or CF[LF], i.e., any of {LF, CR, CRLF}.  The
// next line starts with the next byte after the line terminator.  In other
// words, the line terminator for a given line counts as part of that line,
// not as part of the following line.
class UTF8LineIndex {
 public:
  // Creates a UTF8LineIndex for a file.  The index retains a reference to
  // the file content, which must therefore remain valid (and unchanged) so
  // long as this index is in use.
  //
  // The content must be less than 2GB long.
  //
  // Complexity: O(content.size())
  explicit UTF8LineIndex(std::string_view content);

  // Given a (0-based) byte offset into the file, returns character-based
  // information on the position of that offset.
  //
  // If the offset is greater than the size of the content then this returns
  // an invalid CharacterPosition().
  //
  // Complexity: O(log(#lines) + byte-offset-within-line)
  CharacterPosition ComputePositionForByteOffset(int byte_offset) const;

  // Computes just a (1-based) line number for a given (0-based) byte offset.
  //
  // This is equivalent to ComputePositionForByteOffset(offset).line_number,
  // but more efficient as it doesn't have to compute the column number or
  // character number.
  //
  // Complexity: O(log(#lines))
  int LineNumber(int offset) const;

  // Given a 1-based line and 0-based column, returns the 0-based byte offset
  // into the file.
  //
  // Complexity: O(log(#lines) + column)
  int ComputeByteOffset(int line, int column) const;

  // Returns the number of bytes in a given line.  This includes the bytes
  // of the end-of-line marker, if present.
  int line_size(int line_number) const;

  // Returns a view of the n-th line of the file.
  //
  // The first line is line 1. This returned string_view is a view into the
  // buffer indexed by this UTF8LineIndex.
  std::string_view GetLine(int line_number) const;

  // Returns a substring from the line at a given line number, starting from
  // a given |start_position_in_code_points| and with a length of
  // |length_in_code_points|. Returns an empty string piece if the start
  // position does not exist in the input. If there is not as many code points
  // in the line from the start position as the desired length, returns
  // the rest of the line including the end-of-line marker.
  //
  // TODO: Optimize this function for the case of ASCII.
  std::string_view GetSubstrFromLine(int line_number,
                                     int start_position_in_code_points,
                                     int length_in_code_points) const;

  // Returns the number of lines in the indexed file, including the last line
  // even if it was not terminated.
  int line_count() const {
    // We have three cases:
    // Case 1: empty file.  Considered to have 1 (completely empty) line.
    if (content_.empty()) return 1;
    // Case 2: a file ending in a line-end.  The next character added would
    // be on line n+1, but we're not there yet.
    if (!has_trailing_characters()) {
      return static_cast<int>(line_begin_byte_offsets_.size()) - 1;
    }
    // Case 3: There's an unterminated line at the end of the file.  The
    // next character added would still be on line n.
    return static_cast<int>(line_begin_byte_offsets_.size());
  }

  // Returns (a reference to) the content of the indexed file.
  std::string_view str() const { return content_; }

 private:
  // Populates the index vectors based on content_.
  void IndexContent();

  // Returns whether this file has trailing characters, i.e., characters that
  // are not followed by a newline.  Empty files or file that end in a newline
  // do not have trailing characters, but all other files do.
  bool has_trailing_characters() const {
    return static_cast<size_t>(line_begin_byte_offsets_.back()) !=
           content_.size();
  }

  // The content covered by this UTF8LineIndex.
  std::string_view content_;

  // line_ends_byte_offsets_[n] stores the byte offset of the start of line n-1.
  std::vector<int> line_begin_byte_offsets_;

  // Character offsets corresponding to line_end_byte_offsets_.
  std::vector<int> line_begin_character_offsets_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_UTF8_LINE_INDEX_H_
