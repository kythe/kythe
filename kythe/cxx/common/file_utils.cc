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

#include "file_utils.h"

#include <cstdio>
#include <string>

#include "absl/log/check.h"

namespace kythe {

std::string LoadFileOrDie(const std::string& file) {
  FILE* handle = fopen(file.c_str(), "rb");
  CHECK(handle != nullptr) << "Couldn't open input file " << file;
  CHECK_EQ(fseek(handle, 0, SEEK_END), 0) << "Couldn't seek " << file;
  long size = ftell(handle);
  CHECK_GE(size, 0) << "Bad size for " << file;
  CHECK_EQ(fseek(handle, 0, SEEK_SET), 0) << "Couldn't seek " << file;
  std::string content;
  content.resize(size);
  CHECK_EQ(fread(&content[0], size, 1, handle), 1) << "Couldn't read " << file;
  CHECK_NE(fclose(handle), EOF) << "Couldn't close " << file;
  return content;
}

}  // namespace kythe
