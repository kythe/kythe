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

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

namespace kythe {
namespace lang_textproto {

void ParseRecordioTextChunks(
    std::string content, std::string record_delimiter,
    std::function<void(absl::string_view chunk, int chunk_start_line)>
        callback) {
  absl::string_view trimmed_delimiter =
      absl::StripAsciiWhitespace(record_delimiter);
  std::vector<std::string> lines = absl::StrSplit(content, absl::ByChar('\n'));

  std::ostringstream chunk;
  int currentline = 0, chunk_begin_line = 0;
  // Keep track of whether any lines in the chunk contains textproto data
  // and not all the lines in the chunk are only comment/empty line.
  bool seen_proto_chunk = false;
  for (const auto& line : lines) {
    currentline++;
    const auto& trimmed = absl::StripAsciiWhitespace(line);
    if (seen_proto_chunk && (trimmed == trimmed_delimiter)) {
      LOG(INFO) << "Read chunk at " << chunk_begin_line;
      callback(chunk.str(), chunk_begin_line);

      chunk.str("\0");
      chunk.clear();
      seen_proto_chunk = false;
      chunk_begin_line = currentline;
      continue;
    }
    if (absl::StartsWith(trimmed, "#") || trimmed.empty()) {
      chunk << line << "\n";
      continue;
    }
    // Textproto data, so append it to chunk.
    seen_proto_chunk = true;
    chunk << line << "\n";
  }
  // Last line might contains textproto data.
  if (seen_proto_chunk) {
    callback(chunk.str(), chunk_begin_line);
  }
}

}  // namespace lang_textproto
}  // namespace kythe
