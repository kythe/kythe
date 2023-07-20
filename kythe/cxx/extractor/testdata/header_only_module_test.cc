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
      v_name { path: "kythe/cxx/extractor/testdata/header_only_module.h" }
      info {
        path: "kythe/cxx/extractor/testdata/header_only_module.h"
        digest: "1c1216acd9ba7e73f6e66b5174c9445b191ccefa3fc3a3df3c20cbd931755ca7"
      }
      details {
        [type.googleapis.com/kythe.proto.ContextDependentVersion] {
          row { source_context: "hash1" always_process: false }
        }
      }
    }
    argument: "/dummy/bin/clang"
    argument: "-target"
    argument: "dummy-target"
    argument: "-DKYTHE_IS_RUNNING=1"
    argument: "-resource-dir"
    argument: "/kythe_builtins"
    argument: "-I./kythe/cxx/extractor/testdata"
    argument: "-xc++"
    argument: "-Xclang=-emit-module"
    argument: "-Xclang=-fmodules-embed-all-files"
    argument: "-Xclang=-fmodules-local-submodule-visibility"
    argument: "-fmodules"
    argument: "-fmodule-file-deps"
    argument: "-fno-implicit-modules"
    argument: "-fno-implicit-module-maps"
    argument: "-fmodules-strict-decluse"
    argument: "-fmodule-name=//kythe/cxx/extractor/testdata:header_only_module"
    argument: "-fmodule-map-file=./kythe/cxx/extractor/testdata/header_only_module.cppmap"
    argument: "-Xclang=-fmodule-map-file-home-is-cwd"
    argument: "-fPIE"
    argument: "-fPIC"
    argument: "./kythe/cxx/extractor/testdata/header_only_module.cppmap"
    argument: "-o"
    argument: "header_only_module.pic.pcm"
    argument: "-fsyntax-only"
    source_file: "kythe/cxx/extractor/testdata/header_only_module.cppmap"
    working_directory: "/root"
    entry_context: "hash0"
    output_key: "header_only_module.pic.pcm"
  )pb");
}

TEST(CxxExtractorTest, TestHeaderOnlyModule) {
  CompilationUnit unit = ExtractSingleCompilationOrDie(
      {{"--with_executable",
        "/dummy/bin/clang",
        "-I./kythe/cxx/extractor/testdata",
        "-xc++",
        "-Xclang=-emit-module",
        "-Xclang=-fmodules-embed-all-files",
        "-Xclang=-fmodules-local-submodule-visibility",
        "-fmodules",
        "-fmodule-file-deps",
        "-fno-implicit-modules",
        "-fno-implicit-module-maps",
        "-fmodules-strict-decluse",
        "-fmodule-name=//kythe/cxx/extractor/testdata:header_only_module",
        "-fmodule-map-file=./kythe/cxx/extractor/testdata/"
        "header_only_module.cppmap",
        "-Xclang=-fmodule-map-file-home-is-cwd",
        "-fPIE",
        "-fPIC",
        "-c",
        "./kythe/cxx/extractor/testdata/header_only_module.cppmap",
        "-o",
        "header_only_module.pic.pcm"}});

  // Fix up things which may legitmately vary between runs.
  // TODO(shahms): use gMock protobuf matchers when available.
  CanonicalizeHashes(&unit);
  unit.set_argument(2, "dummy-target");
  unit.clear_details();

  EXPECT_THAT(unit, EquivToProto(ExpectedCompilation()));
}

}  // namespace
}  // namespace kythe
