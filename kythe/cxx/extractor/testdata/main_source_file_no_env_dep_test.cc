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

#include "absl/algorithm/container.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/testlib.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {

using ::kythe::proto::CompilationUnit;
using ::testing::Optional;
using ::testing::SizeIs;

constexpr absl::string_view kExpectedCompilation = R"(
v_name {
  language: "c++"
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"
  }
  info {
    path: "./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"
    digest: "d4fbc426e39f838a6253845a50f0e0089207ed423f852b08c283e3627bc5d49f"
  }
  details {
    [type.googleapis.com/kythe.proto.ContextDependentVersion] {
      row {
        source_context: "hash0"
      }
    }
  }
}
argument: "/dummy/bin/g++"
argument: "-target"
argument: "dummy-target"
argument: "-DKYTHE_IS_RUNNING=1"
argument: "-resource-dir"
argument: "/kythe_builtins"
argument: "--driver-mode=g++"
argument: "-I./kythe/cxx/extractor/testdata"
argument: "-DMACRO"
argument: "./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"
argument: "-fsyntax-only"
source_file: "./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"
working_directory: "TEST_CWD"
entry_context: "hash0"
)";

TEST(CxxExtractorTest, TestSourceFileEnvDepWithoutMacro) {
  absl::optional<std::vector<CompilationUnit>> compilations =
      ExtractCompilations(
          {{"--with_executable", "/dummy/bin/g++",
            "-I./kythe/cxx/extractor/testdata",
            "./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"}});
  ASSERT_THAT(compilations, Optional(SizeIs(1)));

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&compilations->front());
  compilations->front().set_argument(2, "dummy-target");
  compilations->front().set_working_directory("TEST_CWD");
  compilations->front().clear_details();

  CompilationUnit expected =
      ParseTextCompilationUnitOrDie(kExpectedCompilation);
  // The only difference between the two is the lack of "-DMACRO" arguments.
  expected.mutable_argument()->erase(
      absl::c_find(*expected.mutable_argument(), "-DMACRO"));

  EXPECT_THAT(compilations->front(), EquivToCompilation(expected));
}

TEST(CxxExtractorTest, TestSourceFileEnvDepWithMacro) {
  absl::optional<std::vector<CompilationUnit>> compilations =
      ExtractCompilations(
          {{"--with_executable", "/dummy/bin/g++",
            "-I./kythe/cxx/extractor/testdata", "-DMACRO",
            "./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc"}});
  ASSERT_THAT(compilations, Optional(SizeIs(1)));

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&compilations->front());
  compilations->front().set_argument(2, "dummy-target");
  compilations->front().set_working_directory("TEST_CWD");
  compilations->front().clear_details();

  EXPECT_THAT(compilations->front(), EquivToCompilation(kExpectedCompilation));
}

}  // namespace
}  // namespace kythe
