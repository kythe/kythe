/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/textproto/recordio_textparser.h"

#include <sstream>

#include "absl/functional/function_ref.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

namespace kythe {
namespace lang_textproto {

namespace {

// WithChar is a delimiter for absl::StrSplit() that splits on given char but
// also includes the delimiter char.
struct WithChar {
  explicit WithChar(char ch) : delimiter_(ch) {}
  absl::string_view Find(absl::string_view text, size_t pos) const {
    absl::string_view sep = delimiter_.Find(text, pos);
    // Always return a zero-width span after the delimiter, so that it's
    // included if present.
    sep.remove_prefix(sep.size());
    return sep;
  }

 private:
  absl::ByChar delimiter_;
};

void ExtendBy(absl::string_view& data, std::size_t count) {
  data = absl::string_view(data.data(), data.size() + count);
}

}  // namespace

void ParseRecordTextChunks(
    absl::string_view content, absl::string_view record_delimiter,
    absl::FunctionRef<void(absl::string_view chunk, int chunk_start_line)>
        callback) {
  int currentline = 0, chunk_begin_line = 0;
  // Keep track of whether any lines in the chunk contains textproto data
  // and not all the lines in the chunk are only comment/empty line.
  bool seen_proto_chunk = false;
  absl::string_view trimmed_delimiter =
      absl::StripAsciiWhitespace(record_delimiter);
  absl::string_view chunk;
  for (absl::string_view line : absl::StrSplit(content, WithChar('\n'))) {
    currentline++;

    bool initial_chunk = false;
    if (chunk.empty()) {
      chunk = line;
      initial_chunk = true;
    }
    absl::string_view trimmed = absl::StripAsciiWhitespace(line);
    // So far, chunk has textproto data (not just comments or empty lines)
    // and current line is the delimiter.
    if (seen_proto_chunk && (trimmed == trimmed_delimiter)) {
      callback(chunk, chunk_begin_line);

      chunk.remove_prefix(chunk.size());
      seen_proto_chunk = false;
      chunk_begin_line = currentline;
      continue;
    }
    // Empty lines and comments are part of the chunk.
    if (trimmed.empty() || absl::StartsWith(trimmed, "#")) {
      if (!initial_chunk) {
        ExtendBy(chunk, line.size());
      }
      continue;
    }
    // Textproto data, so append it to chunk.
    seen_proto_chunk = true;
    if (!initial_chunk) {
      ExtendBy(chunk, line.size());
    }
  }
  // Last line might contains textproto data.
  if (seen_proto_chunk) {
    callback(chunk, chunk_begin_line);
  }
}

}  // namespace lang_textproto
}  // namespace kythe
