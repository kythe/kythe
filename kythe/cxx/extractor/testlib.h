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

#ifndef KYTHE_CXX_EXTRACTOR_TESTLIB_H_
#define KYTHE_CXX_EXTRACTOR_TESTLIB_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "gmock/gmock.h"
#include "google/protobuf/message.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief Replaces relevant hashes with a unique ID based on visitation order.
void CanonicalizeHashes(kythe::proto::CompilationUnit* unit);

/// \brief Options used during test extraction.
struct ExtractorOptions {
  std::vector<std::string> arguments;
  absl::flat_hash_map<std::string, std::string> environment;
  std::string working_directory;
};

/// \brief Runs the C++ extractor using the provided options and returns the
/// resulting compilations or std::nullopt if there was an error.
std::optional<std::vector<kythe::proto::CompilationUnit>> ExtractCompilations(
    ExtractorOptions options);

/// \brief Runs the C++ extractor using the provided options and returns the
/// resulting CompilationUnit or exits.
kythe::proto::CompilationUnit ExtractSingleCompilationOrDie(
    ExtractorOptions options);

/// \brief Resolves the provided workspace-relative path to find the absolute
/// runfiles path of the given file.
std::optional<std::string> ResolveRunfiles(absl::string_view path);

/// \brief Compares the two protobuf messages with MessageDifferencer and
/// reports the delta.
bool EquivalentCompilations(const kythe::proto::CompilationUnit& lhs,
                            const kythe::proto::CompilationUnit& rhs,
                            std::string* delta);

/// \brief Returns a gMock matcher which compares its argument aginst the
/// provided compilation unit and returns the result.
::testing::Matcher<const kythe::proto::CompilationUnit&> EquivToCompilation(
    const kythe::proto::CompilationUnit& expected);

/// \brief Returns a gMock matcher which compares its argument aginst the
/// provided compilation unit and returns the result.
::testing::Matcher<const kythe::proto::CompilationUnit&> EquivToCompilation(
    absl::string_view expected);

/// \brief Parses a TextFormat CompilationUnit protocol buffer or aborts.
kythe::proto::CompilationUnit ParseTextCompilationUnitOrDie(
    absl::string_view text);

}  // namespace kythe
#endif  // KYTHE_CXX_EXTRACTOR_TESTLIB_H_
