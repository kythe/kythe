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

#ifndef KYTHE_CXX_COMMON_RE2_FLAG_H_
#define KYTHE_CXX_COMMON_RE2_FLAG_H_

#include <memory>
#include <string>

#include <string_view>
#include "re2/re2.h"

namespace kythe {
struct RE2Flag {
  std::shared_ptr<re2::RE2> value;
};
bool AbslParseFlag(std::string_view text, RE2Flag* value, std::string* error);
std::string AbslUnparseFlag(const RE2Flag& value);
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_RE2_FLAG_H_
