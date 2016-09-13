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

#include "objc_bazel_support.h"

#include <llvm/ADT/StringRef.h>
#include "re2/re2.h"

namespace kythe {

std::string RunScript(const std::string &cmd) {
  char buffer[256];
  std::string output = "";
  FILE *f = popen(cmd.c_str(), "r");
  if (!f) {
    return output;
  }
  while (!feof(f)) {
    if (fgets(buffer, 256, f) != NULL) {
      output += buffer;
    }
  }
  pclose(f);
  return llvm::StringRef(output).trim();
}

void FillWithFixedArgs(std::vector<std::string> &args,
                       const blaze::SpawnInfo &si, const std::string &devdir,
                       const std::string &sdkroot) {
  for (auto i : si.argument()) {
    std::string arg = i;
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_DEVELOPER_DIR__", devdir);
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_SDKROOT__", sdkroot);
    args.push_back(arg);
  }
}

}  // namespace kythe
