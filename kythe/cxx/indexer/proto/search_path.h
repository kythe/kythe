/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_PROTO_SEARCH_PATH_H_
#define KYTHE_CXX_INDEXER_PROTO_SEARCH_PATH_H_

#include <string>
#include <vector>

#include "google/protobuf/repeated_field.h"

namespace kythe {
namespace lang_proto {

// Handles --proto_path and -I flags, which specify directories the proto
// compiler should search to find imports. There are two overloads because we
// use this both on command line args (vector) and args from a CompilationUnit
// proto (repeated ptr field).
void ParsePathSubstitutions(
    std::vector<std::string> args,
    std::vector<std::pair<std::string, std::string>>* substitutions,
    std::vector<std::string>* unprocessed);
void ParsePathSubstitutions(
    google::protobuf::RepeatedPtrField<std::string> args,
    std::vector<std::pair<std::string, std::string>>* substitutions,
    std::vector<std::string>* unprocessed);

// Converts a path substitutions map into a list of arguments that can be passed
// to protoc. For example, given the substitution map {"": some/directory}, this
// function would return ["--proto_path", "some/directory"].
std::vector<std::string> PathSubstitutionsToArgs(
    const std::vector<std::pair<std::string, std::string>>& substitutions);

}  // namespace lang_proto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_SEARCH_PATH_H_
