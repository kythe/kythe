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

#include "search_path.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "kythe/cxx/common/path_utils.h"

namespace kythe {
namespace lang_proto {
namespace {

void AddPathSubstitutions(
    absl::string_view path_argument,
    std::vector<std::pair<std::string, std::string>>* substitutions) {
  std::vector<std::string> parts =
      absl::StrSplit(path_argument, ':', absl::SkipEmpty());
  for (const std::string& path_or_substitution : parts) {
    std::string::size_type equals_pos = path_or_substitution.find_first_of('=');
    if (equals_pos == std::string::npos) {
      substitutions->push_back({"", CleanPath(path_or_substitution)});
    } else {
      substitutions->push_back(
          {CleanPath(path_or_substitution.substr(0, equals_pos)),
           CleanPath(path_or_substitution.substr(equals_pos + 1))});
    }
  }
}

template <class ARGLIST>
void _ParsePathSubstitutions(
    ARGLIST args,
    std::vector<std::pair<std::string, std::string>>* substitutions,
    std::vector<std::string>* unprocessed) {
  bool expecting_path_arg = false;
  for (const std::string& argument : args) {
    if (expecting_path_arg) {
      expecting_path_arg = false;
      AddPathSubstitutions(argument, substitutions);
    } else if (argument == "-I" || argument == "--proto_path") {
      expecting_path_arg = true;
    } else {
      absl::string_view argument_value = argument;
      if (absl::ConsumePrefix(&argument_value, "-I")) {
        AddPathSubstitutions(argument_value, substitutions);
      } else if (absl::ConsumePrefix(&argument_value, "--proto_path=")) {
        AddPathSubstitutions(argument_value, substitutions);
      } else if (unprocessed) {
        unprocessed->push_back(argument);
      }
    }
  }
}

}  // namespace

void ParsePathSubstitutions(
    std::vector<std::string> args,
    std::vector<std::pair<std::string, std::string>>* substitutions,
    std::vector<std::string>* unprocessed) {
  _ParsePathSubstitutions(args, substitutions, unprocessed);
}

void ParsePathSubstitutions(
    google::protobuf::RepeatedPtrField<std::string> args,
    std::vector<std::pair<std::string, std::string>>* substitutions,
    std::vector<std::string>* unprocessed) {
  _ParsePathSubstitutions(args, substitutions, unprocessed);
}

std::vector<std::string> PathSubstitutionsToArgs(
    const std::vector<std::pair<std::string, std::string>>& substitutions) {
  std::vector<std::string> args;
  for (const auto& sub : substitutions) {
    // Record in compilation unit args so indexer gets the same paths.
    args.push_back("--proto_path");
    if (sub.first.empty()) {
      args.push_back(sub.second);
    } else {
      args.push_back(absl::StrCat(sub.first, "=", sub.second));
    }
  }
  return args;
}

}  // namespace lang_proto
}  // namespace kythe
