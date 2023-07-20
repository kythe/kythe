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
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/testlib.h"
#include "kythe/proto/analysis.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

namespace kythe {
namespace {
using ::protobuf_matchers::EquivToProto;

proto::CompilationUnit ExpectedCompilation() {
  return ParseTextCompilationUnitOrDie(R"pb(
    v_name { language: "c++" }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/metadata.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/metadata.cc"
        digest: "72269be69625ca9015a59bf7342dce1a30e96ddda51196c9f6ae6c4cbdefb7ea"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash0" always_process: true }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/metadata.cc.meta" }
      info {
        path: "kythe/cxx/extractor/testdata/metadata.cc.meta"
        digest: "1d6faa9e1a76d13f3ab8558a3640158b1f0a54f624a4e37ddc3ef41ed4191058"
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
    argument: "./kythe/cxx/extractor/testdata/metadata.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/metadata.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TextMetadataExtraction) {
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({{
      "--with_executable",
      "/dummy/bin/g++",
      "-I./kythe/cxx/extractor",
      "./kythe/cxx/extractor/testdata/metadata.cc",
  }});
  CanonicalizeHashes(&unit);
  unit.clear_details();
  unit.set_argument(2, "dummy-target");

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}
}  // namespace
}  // namespace kythe
