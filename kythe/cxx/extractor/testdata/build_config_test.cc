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

#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "google/protobuf/message.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/testlib.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/buildinfo.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

namespace kythe {
namespace {
using ::protobuf_matchers::EquivToProto;

proto::CompilationUnit ExpectedCompilation() {
  return ParseTextCompilationUnitOrDie(R"pb(
    v_name { language: "c++" }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/build_config.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/build_config.cc"
        digest: "49e4a60bd04c5ec2070a81f53d7a19db4a3538db6764e5804047c219be5f9309"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash0" always_process: true }
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
    argument: "-I./kythe/cxx/extractor"
    argument: "./kythe/cxx/extractor/testdata/build_config.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/build_config.cc"
    working_directory: "/root"
    entry_context: "hash0"
    details {
      # The TextFormat parser does not like our custom type_url, but generally
      # disregards the part before the type name.
      [type.googleapis.com/kythe.proto.BuildDetails] {
        build_target: "//this/is/a/build:target"
        build_config: "test-build-config"
      }
    }
  )pb");
}

TEST(CxxExtractorTest, TestBuildConfigExtraction) {
  google::protobuf::LinkMessageReflection<kythe::proto::BuildDetails>();
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({
      {
          "--with_executable",
          "/dummy/bin/g++",
          "-I./kythe/cxx/extractor",
          "./kythe/cxx/extractor/testdata/build_config.cc",
      },
      {{"KYTHE_BUILD_CONFIG", "test-build-config"},
       {"KYTHE_ANALYSIS_TARGET", "//this/is/a/build:target"}},
  });
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.mutable_details()->erase(
      std::remove_if(
          unit.mutable_details()->begin(), unit.mutable_details()->end(),
          [&](const auto& any) {
            // This doesn't match the parsed type url above, but is compatible
            // with it.
            return any.type_url() != "kythe.io/proto/kythe.proto.BuildDetails";
          }),
      unit.mutable_details()->end());

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
