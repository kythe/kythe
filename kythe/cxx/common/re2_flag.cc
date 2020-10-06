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

#include "kythe/cxx/common/re2_flag.h"

namespace kythe {
bool AbslParseFlag(absl::string_view text, RE2Flag* value, std::string* error) {
  re2::RE2::Options options;
  options.set_never_capture(true);
  *value = {std::make_shared<re2::RE2>(re2::StringPiece{text.data(), text.size()}, options)};
  if (!value->value->ok()) {
    *error = value->value->error();
  }
  return value->value->ok();
}
std::string AbslUnparseFlag(const RE2Flag& value) {
  return value.value->pattern();
}
}