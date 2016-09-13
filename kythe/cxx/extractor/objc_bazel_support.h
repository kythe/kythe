/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_EXTRACTOR_OBJC_BAZEL_SUPPORT_H_
#define KYTHE_CXX_EXTRACTOR_OBJC_BAZEL_SUPPORT_H_

#include <string>
#include <vector>

#include "third_party/bazel/src/main/protobuf/extra_actions_base.pb.h"

namespace kythe {

/// \brief Run a command and capture its (trimmed) stdout in a string.
std::string RunScript(const std::string &cmd);

// \brief Populate args with the arguments from si where the magic bazel strings
// have been replaced with their actual values.
void FillWithFixedArgs(std::vector<std::string> &args,
                       const blaze::SpawnInfo &si, const std::string &devdir,
                       const std::string &sdkroot);

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_OBJC_BAZEL_SUPPORT_H_
