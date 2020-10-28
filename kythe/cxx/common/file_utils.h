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

#ifndef KYTHE_CXX_COMMON_FILE_UTILS_H_
#define KYTHE_CXX_COMMON_FILE_UTILS_H_

#include <string>

namespace kythe {

/// \brief Loads all data from a file or terminates the process.
std::string LoadFileOrDie(const std::string& file);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_FILE_UTILS_H_
