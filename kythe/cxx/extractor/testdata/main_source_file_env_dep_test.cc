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
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/testlib.h"
#include "kythe/proto/analysis.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

namespace kythe {
namespace {
using ::kythe::proto::CompilationUnit;
using ::protobuf_matchers::EquivToProto;

proto::CompilationUnit ExpectedCompilation() {
  return ParseTextCompilationUnitOrDie(R"pb(
    v_name { language: "c++" }
    required_input {
      v_name {
        path: "kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"
      }
      info {
        path: "kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"
        digest: "7fc8bce46febc9e8c660fdaa7caa9e27e64a9f85769d69ca9ecfc8a2ca751335"
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
    argument: "-I./kythe/cxx/extractor/testdata"
    argument: "-DMACRO"
    argument: "./kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TestSourceFileEnvDepWithoutMacro) {
  CompilationUnit unit = ExtractSingleCompilationOrDie(
      {{"--with_executable", "/dummy/bin/g++",
        "-I./kythe/cxx/extractor/testdata",
        "./kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"}});

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();

  CompilationUnit expected = ExpectedCompilation();
  // The only difference between the two is the lack of "-DMACRO" arguments.
  expected.mutable_argument()->erase(
      absl::c_find(*expected.mutable_argument(), "-DMACRO"));

  EXPECT_THAT(unit, EquivToProto(expected));
}

TEST(CxxExtractorTest, TestSourceFileEnvDepWithMacro) {
  CompilationUnit unit = ExtractSingleCompilationOrDie(
      {{"--with_executable", "/dummy/bin/g++",
        "-I./kythe/cxx/extractor/testdata", "-DMACRO",
        "./kythe/cxx/extractor/testdata/main_source_file_env_dep.cc"}});

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
