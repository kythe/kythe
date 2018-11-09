/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_EXTRACTOR_LANGUAGE_H_
#define KYTHE_CXX_EXTRACTOR_LANGUAGE_H_

#include <string>

#include "llvm/ADT/StringRef.h"

namespace kythe {
namespace supported_language {

// The indexer operates on Objective-C, C, and C++. Since there are links
// between all of those languages, it is easiest if we output all indexer facts
// using a single language, even if it is a bit of a lie.
extern const char* const kIndexerLang;

enum class Language { kCpp, kObjectiveC };

std::string ToString(Language l);

};  // namespace supported_language
};  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_LANGUAGE_H_
