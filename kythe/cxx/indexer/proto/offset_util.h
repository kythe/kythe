/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_PROTO_OFFSET_UTIL_H_
#define KYTHE_CXX_INDEXER_PROTO_OFFSET_UTIL_H_

#include "absl/strings/string_view.h"

namespace kythe {
namespace lang_proto {

// Figures out just how many bytes one needs to go into `line_text` to reach
// what the proto compiler calls column `column_number`.
int ByteOffsetOfTabularColumn(absl::string_view line_text, int column_number);

}  // namespace lang_proto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_OFFSET_UTIL_H_
