/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#include "textproto_schema.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

namespace kythe {
namespace lang_textproto {

TextprotoSchema ParseTextprotoSchemaComments(absl::string_view textproto) {
  TextprotoSchema meta;

  for (absl::string_view line : absl::StrSplit(textproto, "\n")) {
    if (ConsumePrefix(&line, "#")) {
      // Trim space from both ends.
      line = absl::StripAsciiWhitespace(line);

      if (ConsumePrefix(&line, "proto-file:")) {
        meta.proto_file = std::string(StripLeadingAsciiWhitespace(line));
      } else if (ConsumePrefix(&line, "proto-message:")) {
        meta.proto_message = std::string(StripLeadingAsciiWhitespace(line));
      } else if (ConsumePrefix(&line, "proto-import:")) {
        meta.proto_imports.push_back(
            std::string(StripLeadingAsciiWhitespace(line)));
      }
    } else if (line.empty()) {
      // Skip blank lines.
      continue;
    } else {
      // Stop searching after the start of actual textproto content.
      break;
    }
  }

  return meta;
}

}  // namespace lang_textproto
}  // namespace kythe
