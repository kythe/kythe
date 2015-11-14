/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_TOOLS_META_INDEX_UTILS_H_
#define KYTHE_CXX_TOOLS_META_INDEX_UTILS_H_

#include <assert.h>
#include <stdio.h>

#include <memory>

namespace kythe {

struct FileCloser {
  void operator()(FILE* fp) const { assert(::fclose(fp) == 0); }
};

using FilePointer = std::unique_ptr<FILE, FileCloser>;

inline FilePointer OpenFile(const char* path, const char* mode) {
  return FilePointer{::fopen(path, mode)};
}

}  // namespace kythe

#endif  // KYTHE_CXX_TOOLS_META_INDEX_UTILS_H_
