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

#include "offset_util.h"

#include "absl/log/log.h"

namespace kythe {
namespace lang_proto {

int ByteOffsetOfTabularColumn(absl::string_view line_text, int column_number) {
  int computed_column = 0;
  int offset = 0;
  while (computed_column < column_number && offset < line_text.size()) {
    if (line_text[offset] == '\t') {
      // In proto land, tabs go to the next multiple of 8.  There are a million
      // ways of computing this.  This one will do.
      computed_column = (computed_column + 8) - (computed_column % 8);
    } else {
      ++computed_column;
    }
    ++offset;
  }
  if (computed_column != column_number) {
    LOG(ERROR) << "Error computing byte offset: expected " << column_number
               << " columns but counted up to " << computed_column
               << " in line \"" << line_text << "\"";
    return -1;
  }
  return offset;
}

}  // namespace lang_proto
}  // namespace kythe
