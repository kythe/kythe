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
    path: "kythe/cxx/extractor/testdata/arm.cc"
  }
  info {
    path: "./kythe/cxx/extractor/testdata/arm.cc"
    digest: "122b0dc24be3e4d4360ceffdbc00bdd2d6c4ea0600befbad7fbeac1946a9c677"
  }
  details {
    [type.googleapis.com/kythe.proto.ContextDependentVersion] {
      row {
        source_context: "hash0"
      }
    }
  }
}
argument: "arm-none-linux-gnueabi-g++"
argument: "-target"
argument: "armv4t-none-linux-gnueabi"
argument: "-DKYTHE_IS_RUNNING=1"
argument: "-resource-dir"
argument: "/kythe_builtins"
argument: "-target"
argument: "arm-none-linux-gnueabi"
argument: "--driver-mode=g++"
argument: "-Ikythe/cxx/extractor"
argument: "./kythe/cxx/extractor/testdata/arm.cc"
argument: "-fsyntax-only"
source_file: "./kythe/cxx/extractor/testdata/arm.cc"
working_directory: "TEST_CWD"
entry_context: "hash0")";

TEST(CxxExtractorTest, TestAlternatePlatformExtraction) {
  absl::optional<std::vector<CompilationUnit>> compilations =
      ExtractCompilations({{"--with_executable", "arm-none-linux-gnueabi-g++",
                            "-mcpu=cortex-a15", "-Ikythe/cxx/extractor",
                            "./kythe/cxx/extractor/testdata/arm.cc"}});
  ASSERT_THAT(compilations, Optional(SizeIs(1)));

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&compilations->front());
  compilations->front().set_working_directory("TEST_CWD");
  compilations->front().clear_details();

  EXPECT_THAT(compilations->front(), EquivToCompilation(kExpectedCompilation));
}

}  // namespace
}  // namespace kythe
