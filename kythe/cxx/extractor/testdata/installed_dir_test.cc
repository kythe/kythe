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
      v_name { path: "kythe/cxx/extractor/testdata/installed_dir.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/installed_dir.cc"
        digest: "fb21e5e71e3b8f54ebf5a84fc2955fb4f893111fdfa0d7fe8be936b32add3f56"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row {
            source_context: "hash0"
            always_process: true
            column { linked_context: "hash1" }
          }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/include/c++/v1/dummy" }
      info {
        # TODO(b/278254885): Properly normalize file paths.
        path: "kythe/cxx/extractor/testdata/include/c++/v1/dummy"
        digest: "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash1" }
        }
      }
    }
    argument: "kythe/cxx/extractor/testdata/bin/clang++"
    argument: "-target"
    argument: "dummy-target"
    argument: "-DKYTHE_IS_RUNNING=1"
    argument: "-resource-dir"
    argument: "/kythe_builtins"
    argument: "--driver-mode=g++"
    argument: "-stdlib=libc++"
    argument: "-v"
    argument: "./kythe/cxx/extractor/testdata/installed_dir.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/installed_dir.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TestInstalledClangExtraction) {
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({{
      "--with_executable",
      "kythe/cxx/extractor/testdata/bin/clang++",
      "-stdlib=libc++",
      "-E",
      "-v",  // On failure, dump the search path.
      "./kythe/cxx/extractor/testdata/installed_dir.cc",
  }});
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
