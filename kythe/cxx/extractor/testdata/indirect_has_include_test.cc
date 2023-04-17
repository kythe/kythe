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
      v_name { path: "kythe/cxx/extractor/testdata/indirect_has_include.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/indirect_has_include.cc"
        digest: "6b406c26560e435ba485a141440055fc21c55492b41653003e13715292b7d76d"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row {
            source_context: "hash0"
            always_process: true
            column { offset: 35 linked_context: "hash1" }
          }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/has_include.h" }
      info {
        path: "kythe/cxx/extractor/testdata/has_include.h"
        digest: "ebebe3a0bf6fb1d21593bcf52d899124ea175ac04eae16a366ed0b9220ae0d06"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash2" }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/indirect_has_include.h" }
      info {
        path: "kythe/cxx/extractor/testdata/indirect_has_include.h"
        digest: "e92d9e48bed6844a99ebc827f38300af75762c2c2516b98dc560e680568a677a"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row {
            source_context: "hash1"
            column { linked_context: "hash2" }
          }
        }
      }
    }
    argument: "/dummy/bin/clang++"
    argument: "-target"
    argument: "dummy-target"
    argument: "-DKYTHE_IS_RUNNING=1"
    argument: "-resource-dir"
    argument: "/kythe_builtins"
    argument: "--driver-mode=g++"
    argument: "-I./kythe/cxx/extractor"
    argument: "./kythe/cxx/extractor/testdata/indirect_has_include.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/indirect_has_include.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TextHasIncludeExtraction) {
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({{
      "--with_executable",
      "/dummy/bin/clang++",
      "-I./kythe/cxx/extractor",
      "./kythe/cxx/extractor/testdata/indirect_has_include.cc",
  }});
  CanonicalizeHashes(&unit);
  unit.clear_details();
  unit.set_argument(2, "dummy-target");

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
