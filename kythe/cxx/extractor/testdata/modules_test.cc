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
#include "absl/strings/match.h"
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
      v_name { path: "kythe/cxx/extractor/testdata/modules.cc" }
      info {
        path: "kythe/cxx/extractor/testdata/modules.cc"
        digest: "65f89e233c735e33c51ab2445b0586b102d03430d3ed177e021a33b1fffeac9f"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash0" always_process: true }
        }
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/modfoo.h" }
      info {
        path: "kythe/cxx/extractor/testdata/modfoo.h"
        digest: "cd8c82152d321ecfa60a7ab3ccec78ad7168abf22dd93150ccee7fea420ad432"
      }
    }
    required_input {
      v_name { path: "kythe/cxx/extractor/testdata/modfoo.modulemap" }
      info {
        path: "kythe/cxx/extractor/testdata/modfoo.modulemap"
        digest: "94416d419ecab4bce80084ef89587c4ca31a3cabad12498bd98aa213b1dd5189"
      }
    }
    argument: "/dummy/bin/g++"
    argument: "-target"
    argument: "dummy-target"
    argument: "-DKYTHE_IS_RUNNING=1"
    argument: "-resource-dir"
    argument: "/kythe_builtins"
    argument: "--driver-mode=g++"
    argument: "-fmodules"
    argument: "-fmodule-map-file=kythe/cxx/extractor/testdata/modfoo.modulemap"
    argument: "-I./kythe/cxx/extractor"
    argument: "./kythe/cxx/extractor/testdata/modules.cc"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/modules.cc"
    working_directory: "/root"
    entry_context: "hash0"
  )pb");
}

TEST(CxxExtractorTest, TestModulesExtraction) {
  kythe::proto::CompilationUnit unit = ExtractSingleCompilationOrDie({{
      "--with_executable",
      "/dummy/bin/g++",
      "-fmodules",
      "-fmodule-map-file=kythe/cxx/extractor/testdata/modfoo.modulemap",
      absl::StrCat("-fmodules-cache-path=", testing::TempDir(),
                   "/module-cache"),
      "-I./kythe/cxx/extractor",
      "./kythe/cxx/extractor/testdata/modules.cc",
  }});
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();
  unit.mutable_argument()->erase(
      absl::c_find_if(unit.argument(), [](absl::string_view arg) {
        return absl::StartsWith(arg, "-fmodules-cache-path=");
      }));

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
