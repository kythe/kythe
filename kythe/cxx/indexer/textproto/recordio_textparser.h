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

#ifndef KYTHE_CXX_INDEXER_TEXTPROTO_RECORDIO_TEXTPARSER_H_
#define KYTHE_CXX_INDEXER_TEXTPROTO_RECORDIO_TEXTPARSER_H_

#include <optional>

#include "absl/functional/function_ref.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/text_format.h"

namespace kythe {
namespace lang_textproto {

// Parses recordio text-formatted content separated by record delimiter.
// Callback is invoked for each chunk that is parsed into proto message along
// with the line offset of where that chunk begins in the file.
//
// Empty lines can be record delimiter and they can also separate two chunks
// of comments. Always gather chunk of comments into one big chunk along
// with next message. For example:
// 1.  # FooBar
// 2.  # Bar
// 3.
// 4.  # BarBar
// 5.  name: "hello"
//
// In the above example, line 1-5 should be read as one chunk even if the
// delimiter is empty line. Also, note that last record usually does not have
// delimiter specified and the record ends at EOF.
// Also, note that delimiter could start with '#' which is also for the
// comment.
void ParseRecordTextChunks(
    absl::string_view content, absl::string_view record_delimiter,
    absl::FunctionRef<void(absl::string_view chunk, int chunk_start_line)>
        callback);

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_RECORDIO_TEXTPARSER_H_
