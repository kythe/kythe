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

#include <algorithm>
#include <cstring>
#include <string>

#include "absl/log/check.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "gtest/gtest.h"

namespace {

using ::kythe::CharacterPosition;
using ::kythe::UTF8LineIndex;

// Returns whether a UTF-8 byte is a continuation byte, i.e., a byte other
// than the first byte of the encoding of a character.
static bool IsUTF8ContinuationByte(int byte) { return ((byte & 0xC0) == 0x80); }

// Checks that round-tripping is a no-op for the first byte of each character
// covered by a UTF8LineIndex, and also for breaking the file up into lines
// and then concatenating those lines.
void CheckRoundTrips(const UTF8LineIndex& index) {
  absl::string_view content(index.str());
  for (int byte_offset = 0; byte_offset < content.size(); ++byte_offset) {
    if (IsUTF8ContinuationByte(content[byte_offset])) continue;
    CharacterPosition position(index.ComputePositionForByteOffset(byte_offset));
    EXPECT_TRUE(position.is_valid())
        << " at byte offset: " << byte_offset << " in " << content;
    EXPECT_EQ(byte_offset, index.ComputeByteOffset(position.line_number,
                                                   position.column_number));
  }

  std::string joined_lines;
  for (int line_number = 1; line_number <= index.line_count(); ++line_number) {
    absl::StrAppend(&joined_lines, index.GetLine(line_number));
  }
  EXPECT_EQ(index.str(), joined_lines);
}

TEST(UTF8LineIndexTest, WorksForAnEmptyFile) {
  // All we expect for an empty input is that this doesn't crash, reports
  // that an empty file contains one line, gives a sane response for the
  // past-the-end position, and handles bad requests robustly.
  const std::string empty_file_content("");
  UTF8LineIndex index(empty_file_content);

  EXPECT_EQ(1, index.line_count());

  EXPECT_FALSE(index.ComputePositionForByteOffset(1).is_valid());
  EXPECT_FALSE(index.ComputePositionForByteOffset(127).is_valid());

  CheckRoundTrips(index);

  CharacterPosition past_the_end = index.ComputePositionForByteOffset(0);
  EXPECT_TRUE(past_the_end.is_valid());
  EXPECT_EQ(1, past_the_end.line_number);
  EXPECT_EQ(0, past_the_end.column_number);
  EXPECT_EQ(0, past_the_end.character_number);
}

TEST(UTF8LineIndexTest, WorksForSingleLineAsciiFile) {
  const std::string single_line_content("Hello World!\n");
  UTF8LineIndex index(single_line_content);
  // The whole "file" is on line 1.
  for (int i = 0; i < single_line_content.size(); ++i) {
    EXPECT_EQ(1, index.LineNumber(i)) << "i = " << i;
    EXPECT_EQ(i, index.ComputePositionForByteOffset(i).column_number);
  }
  EXPECT_EQ(1, index.line_count());
  CheckRoundTrips(index);

  auto past_the_end = index.ComputePositionForByteOffset(index.str().size());
  EXPECT_TRUE(past_the_end.is_valid());
  EXPECT_EQ(2, past_the_end.line_number);
  EXPECT_EQ(0, past_the_end.column_number);
}

TEST(UTF8LineIndexTest, WorksForFileWithEmptyFirstLine) {
  const std::string content_with_empty_first_line("\nSecond line\n");
  UTF8LineIndex index(content_with_empty_first_line);
  // The initial newline counts as part of line 1.
  EXPECT_EQ(1, index.LineNumber(0));
  // The rest of the file is on line 2.
  for (int i = 1; i < content_with_empty_first_line.size(); ++i) {
    EXPECT_EQ(2, index.LineNumber(i));
  }
  CheckRoundTrips(index);
}

// Tests lines terminated with just CR (0x0D).
TEST(UTF8LineIndexTest, WorksForMacStyleLineEnds) {
  const std::string mac_style_file_content("Mac\rStyle\rLines\r");
  UTF8LineIndex index(mac_style_file_content);
  CheckRoundTrips(index);
}

TEST(UTF8LineIndexTest, WorksForPlainASCIIFile) {
  const std::string ascii_content("Now is the {\nWinter of}\nyour disc\n");
  UTF8LineIndex index(ascii_content);
  CheckRoundTrips(index);

  // Some tests for the first 'o' character.
  EXPECT_EQ(1, index.LineNumber(1));
  EXPECT_EQ(1, index.ComputePositionForByteOffset(1).line_number);
  EXPECT_EQ(1, index.ComputePositionForByteOffset(1).column_number);

  // There's only one 'c' in ascii_content.  We depend on this below, so
  // check it here.  This is a CHECK rather than EXPECT or ASSERT because
  // it's not an assertion about the code under test, it's an assertion
  // about this test code.
  CHECK_EQ(1, std::count(ascii_content.begin(), ascii_content.end(), 'c'));
  // 'c' is the 9th character (hence, column 8) of the 3rd line ("your disc").
  auto position_of_c =
      index.ComputePositionForByteOffset(ascii_content.find('c'));
  EXPECT_EQ(3, position_of_c.line_number);
  EXPECT_EQ(8, position_of_c.column_number);
  EXPECT_EQ(ascii_content.find('c'),
            index.ComputeByteOffset(position_of_c.line_number,
                                    std::string("your disc").find('c')));
}

TEST(UTF8LineIndexTest, WorksForFileWithMissingTerminalLineEnd) {
  const std::string ascii_content("Now is the {\nWinter of}\nyour disc");
  UTF8LineIndex index(ascii_content);
  CheckRoundTrips(index);
  EXPECT_EQ(3, index.line_count());
  EXPECT_EQ(1, index.LineNumber(1));
  EXPECT_EQ(1, index.ComputePositionForByteOffset(1).line_number);
  EXPECT_EQ(3, index.LineNumber(ascii_content.size() - 2));
  EXPECT_EQ(3, index.line_count());

  auto past_the_end = index.ComputePositionForByteOffset(index.str().size());
  EXPECT_TRUE(past_the_end.is_valid());
  EXPECT_EQ(3, past_the_end.line_number);
  EXPECT_EQ(9, past_the_end.column_number);

  EXPECT_FALSE(index.ComputePositionForByteOffset(12345678).is_valid());

  EXPECT_EQ(13, index.line_size(1));
  EXPECT_EQ(11, index.line_size(2));
  EXPECT_EQ(9, index.line_size(3));
}

// TODO(jdennett): Split WorksWithDoubleByteCharacters out into smaller tests.
TEST(UTF8LineIndexTest, WorksWithDoubleByteCharacters) {
  const std::string text_with_double_byte_characters =
      "$1 = £0.6354\n"
      "£1 = $1.5739\n";
  UTF8LineIndex index(text_with_double_byte_characters);
  CheckRoundTrips(index);

  // The "=" on the first line is at byte offset 3 and (0-based) column 3.
  EXPECT_EQ(3, index.ComputePositionForByteOffset(3).column_number);

  auto first_pound_position =
      index.ComputePositionForByteOffset(5);  // the first byte of £
  EXPECT_EQ(1, first_pound_position.line_number);
  EXPECT_EQ(5, first_pound_position.column_number);

  auto after_pound_position =
      index.ComputePositionForByteOffset(7);  // the first byte *after* £
  EXPECT_EQ(1, after_pound_position.line_number);
  EXPECT_EQ(6, after_pound_position.column_number);  // 8th byte, 7th character

  // Each of the two lines is 13 characters wrong including the LF control
  // character, but one pound character in each line is represented as two
  // bytes in UTF-8, so each lines is 14 bytes in total.
  EXPECT_EQ(14, index.line_size(1));
  EXPECT_EQ(14, index.line_size(2));

  // Line 1 starts at character index 0.
  auto start_of_line_1 =
      index.ComputePositionForByteOffset(index.ComputeByteOffset(1, 0));
  EXPECT_EQ(1, start_of_line_1.line_number);
  EXPECT_EQ(0, start_of_line_1.column_number);
  EXPECT_EQ(0, start_of_line_1.character_number);

  // Line 2 starts at character index 13.
  auto start_of_line_2 =
      index.ComputePositionForByteOffset(index.ComputeByteOffset(2, 0));
  EXPECT_EQ(2, start_of_line_2.line_number);
  EXPECT_EQ(0, start_of_line_2.column_number);
  EXPECT_EQ(13, start_of_line_2.character_number);
}

TEST(UTF8LineIndexTest, CRLFIsASingleLineEnd) {
  const std::string four_empty_lines(
      "\n"
      "\r"
      "\r\n"
      "\r\n");
  UTF8LineIndex index(four_empty_lines);
  CheckRoundTrips(index);

  EXPECT_EQ(4, index.line_count());
  EXPECT_EQ(1, index.line_size(1));
  EXPECT_EQ(1, index.line_size(2));
  EXPECT_EQ(2, index.line_size(3));
  EXPECT_EQ(2, index.line_size(4));
}

TEST(UTF8LineIndexTest, KeepFrombergerHappy) {
  const std::string michael("abc\r\ndef");
  UTF8LineIndex index(michael);
  CheckRoundTrips(index);

  // The first line consists of five bytes: "abc\r\n".
  EXPECT_EQ(5, index.ComputeByteOffset(2, 0));
  EXPECT_EQ(5, index.line_size(1));
  EXPECT_EQ(3, index.line_size(2));
}

TEST(UTF8LineIndexTest, GetLineFromEmptyFile) {
  const std::string empty_file;
  UTF8LineIndex empty_index(empty_file);
  EXPECT_EQ("", empty_index.GetLine(-1));
  EXPECT_EQ("", empty_index.GetLine(0));
  EXPECT_EQ("", empty_index.GetLine(1));
  EXPECT_EQ("", empty_index.GetLine(2));
  EXPECT_EQ("", empty_index.GetLine(999));
}

TEST(UTF8LineIndexTest, GetLineFromUnterminatedFile) {
  const std::string unterminated_file(
      "Hello world.\n"
      "Goodbye, unterminated world.");
  UTF8LineIndex unterminated_index(unterminated_file);
  CheckRoundTrips(unterminated_index);

  EXPECT_EQ("", unterminated_index.GetLine(0));
  EXPECT_EQ("Hello world.\n", unterminated_index.GetLine(1));
  EXPECT_EQ("Goodbye, unterminated world.", unterminated_index.GetLine(2));
  EXPECT_EQ("", unterminated_index.GetLine(3));
}

TEST(UTF8LineIndexTest, GetLineFromSmallFile) {
  const std::string small_file("\nline two\nline three\r\n");
  UTF8LineIndex small_index(small_file);
  CheckRoundTrips(small_index);

  EXPECT_EQ("", small_index.GetLine(-1));
  EXPECT_EQ("", small_index.GetLine(0));
  EXPECT_EQ("\n", small_index.GetLine(1));
  EXPECT_EQ("line two\n", small_index.GetLine(2));
  EXPECT_EQ("line three\r\n", small_index.GetLine(3));
  EXPECT_EQ("", small_index.GetLine(4));
  EXPECT_EQ("", small_index.GetLine(5));
  EXPECT_EQ("", small_index.GetLine(999));
}

TEST(UTF8LineIndexTest, ComputeByteOffsetAtEndOfUnterminatedFile) {
  // This is a regression test; migrating from ::string storage to using a
  // string_view means that peeking past the end of the buffer isn't allowed
  // anymore, and this aborted in ComputeByteOffset previously.
  const std::string unterminated_file(
      "Hello world.\n"
      "Goodbye, unterminated world.");
  UTF8LineIndex index(unterminated_file);
  EXPECT_EQ(unterminated_file.length(),
            index.ComputeByteOffset(2, strlen("Goodbye, unterminated world.")));
}

}  // anonymous namespace
