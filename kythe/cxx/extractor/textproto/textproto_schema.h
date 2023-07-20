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

#ifndef KYTHE_CXX_EXTRACTOR_TEXTPROTO_SCHEMA_H_
#define KYTHE_CXX_EXTRACTOR_TEXTPROTO_SCHEMA_H_

#include <string>
#include <string_view>
#include <vector>

namespace kythe {
namespace lang_textproto {

struct TextprotoSchema {
  std::string_view proto_message, proto_file;
  std::vector<std::string_view> proto_imports;
};

/// Parses comments at the top of textproto files that specify the corresponding
/// proto message and which file that message comes from. The "proto-file" and
/// "proto-message" lines are generally required, while "proto-import" is
/// optional and only needed if the textproto uses extensions that aren't
/// already imported by "proto-file". The comments should be of the form:
///
///   # proto-file: some/file.proto
///   # proto-message: some_namespace.MyMessage
///   # proto-import: some/other/file.proto
///   # proto-import: file/with/extensions.proto
///
/// \param textproto the contents of the textproto file
/// \return the schema information seen in the textproto. May be
/// incomplete.
/// WARNING: The return value is made up of string_views that reference the
/// input. Therefore the input string_view must outlive any use of the output.
TextprotoSchema ParseTextprotoSchemaComments(std::string_view textproto);

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_TEXTPROTO_SCHEMA_H_
