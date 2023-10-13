/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include <stdio.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "google/protobuf/repeated_ptr_field.h"
#include "llvm/ADT/StringRef.h"
#include "re2/re2.h"
#include "third_party/bazel/src/main/protobuf/extra_actions_base.pb.h"

namespace kythe {

// This is inspiried by Python's commands.mkarg
// https://hg.python.org/cpython/file/tip/Lib/commands.py#l81
std::string SanitizeArgument(const std::string& s) {
  if (s.find('\'') == std::string::npos) {
    // There are no single quotes, so we can make the string safe by putting it
    // in single quotes.
    return "'" + s + "'";
  }
  // There are single quotes in the string, so we will wrap it in double quotes
  // and escape backslash, double-quote, dollar, and back-tick.
  std::string ret = s;
  RE2::GlobalReplace(&ret, R"(([\\"$`]))", R"(\\\1)");
  return "\"" + ret + "\"";
}

std::string BuildEnvVarCommandPrefix(
    const google::protobuf::RepeatedPtrField<blaze::EnvironmentVariable>&
        vars) {
  std::stringstream ret;
  for (const auto& env : vars) {
    // Neither name nor value are validated or sanitized.
    // This really should be unnecessary, but we don't have any guarantees.
    if (RE2::FullMatch(env.name(), "[a-zA-Z_][a-zA-Z_0-9]*")) {
      ret << env.name() << "=" << SanitizeArgument(env.value()) << " ";
    }
  }
  return ret.str();
}

std::string RunScript(const std::string& cmd) {
  char buffer[256];
  std::string output = "";
  FILE* f = popen(cmd.c_str(), "r");
  if (!f) {
    return output;
  }
  while (!feof(f)) {
    if (fgets(buffer, 256, f) != nullptr) {
      output += buffer;
    }
  }
  pclose(f);
  return std::string(llvm::StringRef(output).trim());
}

void FillWithFixedArgs(std::vector<std::string>& args,
                       const blaze::CppCompileInfo& ci,
                       const std::string& devdir, const std::string& sdkroot) {
  args.push_back(ci.tool());
  for (const auto& i : ci.compiler_option()) {
    std::string arg = i;
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_DEVELOPER_DIR__", devdir);
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_SDKROOT__", sdkroot);
    args.push_back(arg);
  }
  // If the command-line did not specify "-c x.m" specifically, include the
  // primary source at the end of the argument list.
  if (std::find(args.begin(), args.end(), "-c") == args.end()) {
    args.push_back(ci.source_file());
  }
}

void FillWithFixedArgs(std::vector<std::string>& args,
                       const blaze::SpawnInfo& si, const std::string& devdir,
                       const std::string& sdkroot) {
  for (const auto& i : si.argument()) {
    std::string arg = i;
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_DEVELOPER_DIR__", devdir);
    RE2::GlobalReplace(&arg, "__BAZEL_XCODE_SDKROOT__", sdkroot);
    args.push_back(arg);
  }
}

}  // namespace kythe
