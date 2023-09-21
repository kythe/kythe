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
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"

namespace kythe {
namespace lang_textproto {

TextprotoSchema ParseTextprotoSchemaComments(absl::string_view textproto) {
  TextprotoSchema schema;

  for (absl::string_view line : absl::StrSplit(textproto, '\n')) {
    if (absl::ConsumePrefix(&line, "#")) {
      line = absl::StripLeadingAsciiWhitespace(line);
      if (absl::ConsumePrefix(&line, "proto-file:")) {
        schema.proto_file = absl::StripAsciiWhitespace(line);
      } else if (absl::ConsumePrefix(&line, "proto-message:")) {
        schema.proto_message = absl::StripAsciiWhitespace(line);
      } else if (absl::ConsumePrefix(&line, "proto-import:")) {
        schema.proto_imports.push_back(absl::StripAsciiWhitespace(line));
      }
    } else if (line.empty()) {
      // Skip blank lines.
      continue;
    } else {
      // Stop searching after the start of actual textproto content.
      break;
    }
  }

  return schema;
}

}  // namespace lang_textproto
}  // namespace kythe
