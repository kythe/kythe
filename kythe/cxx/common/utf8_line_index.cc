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

#include "kythe/cxx/common/utf8_line_index.h"

#include <cstdint>
#include <ostream>

#include "absl/algorithm/container.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"

namespace kythe {

std::ostream& operator<<(std::ostream& dest,
                         const CharacterPosition& position) {
  dest << "<line_number=" << position.line_number
       << " column_number=" << position.column_number
       << " character_number=" << position.character_number << ">";
  return dest;
}

bool IsUTF8ContinuationByte(int byte) { return ((byte & 0xC0) == 0x80); }

bool IsUTF8EndOfLineByte(int byte_offset, absl::string_view content) {
  // If/when we were using a string, checking for the past-the-end byte
  // was safe.  Now that we use string_view we have to avoid that.
  return (content[byte_offset] == '\n' ||
          (content[byte_offset] == '\r' && (byte_offset + 1 == content.size() ||
                                            content[byte_offset + 1] != '\n')));
}

UTF8LineIndex::UTF8LineIndex(absl::string_view content) : content_(content) {
  IndexContent();
}

void UTF8LineIndex::IndexContent() {
  CHECK_LT(content_.size(), 1LL << 32);

  // Line 0 starts at offset 0.  All other line start offsets are determined
  // by scanning for line ends (any of {CR, CR+LF, LF}.)
  line_begin_byte_offsets_ = {0};
  line_begin_character_offsets_ = {0};

  int character_number = 0;
  for (int byte_offset = 0; byte_offset < content_.size(); ++byte_offset) {
    if (IsUTF8ContinuationByte(content_[byte_offset])) continue;

    if (IsUTF8EndOfLineByte(byte_offset, content_)) {
      line_begin_byte_offsets_.push_back(byte_offset + 1);
      line_begin_character_offsets_.push_back(character_number + 1);
    }
    ++character_number;
  }
}

CharacterPosition UTF8LineIndex::ComputePositionForByteOffset(
    int byte_offset) const {
  CharacterPosition position;

  // Error cases: before the start of the file, or after the past-the-end
  // position.  In either of those cases we return an invalid position.
  if (byte_offset < 0 || byte_offset > static_cast<int64_t>(content_.size()))
    return position;

  // We special-case asking for a position at the start of the file in part
  // because it allows us to ignore the case of an empty file later.
  if (byte_offset == 0) {
    position.line_number = 1;
    position.column_number = 0;
    position.character_number = 0;
    return position;
  }

  if (byte_offset < static_cast<int64_t>(content_.size())) {
    position.line_number = LineNumber(byte_offset);
    auto line_begin_byte_offset =
        line_begin_byte_offsets_[position.line_number - 1];
    // Count the characters in the line up to (and including) byte_offset.
    position.column_number = -1;
    for (int offset = line_begin_byte_offset; offset <= byte_offset; ++offset) {
      if (!IsUTF8ContinuationByte(content_[offset])) ++position.column_number;
    }
    auto line_begin_character_offset =
        line_begin_character_offsets_[position.line_number - 1];
    position.character_number =
        line_begin_character_offset + position.column_number;
  } else if (byte_offset == content_.size()) {
    // TODO(unknown): see if we can unify this with the previous case
    // in a less ugly manner.
    position = ComputePositionForByteOffset(byte_offset - 1);
    // For the past-the-end position, we want it to be on the same line as
    // any trailing characters, or on a new line if there are none.  (We've
    // taken care of the empty file case elsewhere.)
    if (has_trailing_characters()) {
      ++position.column_number;
      ++position.character_number;
    } else {
      ++position.line_number;
      position.column_number = 0;
      position.character_number = 0;
    }
  }
  return position;
}

int UTF8LineIndex::LineNumber(int byte_offset) const {
  if (content_.empty()) return 1;
  const auto next_line =
      absl::c_upper_bound(line_begin_byte_offsets_, byte_offset);
  return next_line - line_begin_byte_offsets_.begin();
}

int UTF8LineIndex::line_size(int line_number) const {
  if (line_number == line_count()) {
    return content_.size() - ComputeByteOffset(line_number, 0);
  } else {
    return (ComputeByteOffset(line_number + 1, 0) -
            ComputeByteOffset(line_number, 0));
  }
}

int UTF8LineIndex::ComputeByteOffset(int line, int column) const {
  if (line < 1 || line > line_begin_byte_offsets_.size()) return -1;
  int byte_offset = line_begin_byte_offsets_[line - 1];
  for (int i = 0; i < column; ++i) {
    // Skip over one character for each column, however many bytes that is.
    // In other words: skip the first byte and then skip any continuation bytes.
    ++byte_offset;
    while ((byte_offset < static_cast<int64_t>(content_.size())) &&
           IsUTF8ContinuationByte(content_[byte_offset])) {
      ++byte_offset;
    }
  }
  return byte_offset;
}

absl::string_view UTF8LineIndex::GetLine(int line_number) const {
  const int start_byte_offset = ComputeByteOffset(line_number, 0);
  if (line_number == line_begin_byte_offsets_.size()) {
    // The last line is a special case, as it might be unterminated.
    return absl::ClippedSubstr(content_, start_byte_offset);
  }
  const int end_byte_offset = ComputeByteOffset(line_number + 1, 0);
  if (start_byte_offset == -1 || end_byte_offset == -1) {
    // Error case.
    return absl::string_view();
  }
  return absl::ClippedSubstr(content_, start_byte_offset,
                             end_byte_offset - start_byte_offset);
}

absl::string_view UTF8LineIndex::GetSubstrFromLine(
    int line_number, int start_position_in_code_points,
    int length_in_code_points) const {
  CHECK_GE(start_position_in_code_points, 0);
  CHECK_GE(length_in_code_points, 0);

  absl::string_view line = GetLine(line_number);

  int start_byte_offset = -1;  // a negative number means not being set yet
  int code_point_number = 0;
  int byte_offset = 0;  // will point to the end of substr
  for (; byte_offset < line.size(); ++byte_offset) {
    if (code_point_number == start_position_in_code_points &&
        // Set the start offset only once in case the start code point is
        // longer than 1 byte
        start_byte_offset == -1) {
      start_byte_offset = byte_offset;
    }
    if (code_point_number ==
        start_position_in_code_points + length_in_code_points) {
      break;
    }
    if (IsUTF8ContinuationByte(line[byte_offset])) continue;
    ++code_point_number;
  }

  if (start_byte_offset >= 0) {
    CHECK_GE(byte_offset, start_byte_offset);
    return absl::ClippedSubstr(line, start_byte_offset,
                               byte_offset - start_byte_offset);
  } else {
    LOG(ERROR) << "Substring index " << start_position_in_code_points
               << " not found in " << line;
    return absl::string_view();
  }
}

}  // namespace kythe
