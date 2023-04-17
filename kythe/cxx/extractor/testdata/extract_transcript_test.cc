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
      v_name { path: "kythe/cxx/extractor/testdata/transcript_main.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/transcript_main.cc"
        digest: "a719ebca696362eb639992c5afd3b08ab6c1938bc5639fdfe1fde06fd86bd622"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row {
            source_context: "hash0"
            always_process: true
            column { offset: 20 linked_context: "hash1" }
            column { offset: 46 linked_context: "hash2" }
            column { offset: 72 linked_context: "hash3" }
            column { offset: 98 linked_context: "hash2" }
          }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/transcript_a.h" }
      info {
        path: "kythe/cxx/extractor/testdata/transcript_a.h"
        digest: "15d3490610af31dff6f1be9948ef61e66db48a84fc8fd93a81a5433abab04309"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash3" }
          row { source_context: "hash1" }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/transcript_b.h" }
      info {
        path: "kythe/cxx/extractor/testdata/transcript_b.h"
        digest: "6904079b7b9d5d0586a08dbbdac2b08d35f00c1dbb4cc63721f22a347f52e2f7"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash2" }
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
    argument: "./kythe/cxx/extractor/testdata/transcript_main.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/transcript_main.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TextExtractedTranscript) {
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({{
      "--with_executable",
      "/dummy/bin/g++",
      "-I./kythe/cxx/extractor/testdata",
      "./kythe/cxx/extractor/testdata/transcript_main.cc",
  }});
  CanonicalizeHashes(&unit);
  unit.clear_details();
  unit.set_argument(2, "dummy-target");

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
