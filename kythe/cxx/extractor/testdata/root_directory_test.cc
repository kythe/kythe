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

#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/extractor/testlib.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {
using ::testing::SizeIs;
using ::kythe::proto::CompilationUnit;

constexpr absl::string_view kFilePath =
    "kythe/cxx/extractor/testdata/altroot/altpath/file.cc";

constexpr absl::string_view kExpectedCompilation = R"(
v_name {
  language: "c++"
}
required_input {
  v_name {
    path: "altpath/file.cc"
  }
  info {
    path: "file.cc"
    digest: "a24091884bc15b53e380fe5b874d1bb52d89269fdf2592808ac70ba189204730"
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
argument: "file.cc"
argument: "-fsyntax-only"
source_file: "file.cc"
working_directory: "/root"
entry_context: "hash0")";

// Verifies that the extractor properly handles KYTHE_ROOT_DIRECTORY
// other than the working directory.
TEST(RootDirectoryTest, AlternateRootDirectoryExtracts) {
  absl::optional<std::string> resolved_path = ResolveRunfiles(kFilePath);
  ASSERT_TRUE(resolved_path.has_value());

  std::string filename(Basename(*resolved_path));
  std::string working_directory(Dirname(*resolved_path));
  std::string root_directory(Dirname(working_directory));

  ExtractorOptions extraction;
  extraction.working_directory = working_directory;
  extraction.arguments = {"--with_executable", "/dummy/bin/g++", filename};
  extraction.environment = {
      {"KYTHE_ROOT_DIRECTORY", root_directory},
  };

  CompilationUnit unit = ExtractSingleCompilationOrDie(std::move(extraction));
  ASSERT_THAT(unit.argument(), SizeIs(9));

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();

  EXPECT_THAT(unit, EquivToCompilation(kExpectedCompilation));
}

}  // namespace
}  // namespace kythe
