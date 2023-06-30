/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// cxx_extractor_bazel is a C++ extractor meant to be run as a Bazel
// extra_action.

#include <fcntl.h>
#include <sys/stat.h>

#include <fstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "cxx_extractor.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/extractor/language.h"
#include "third_party/bazel/src/main/protobuf/extra_actions_base.pb.h"

ABSL_FLAG(std::string, build_config, "",
          "Human readable description of the build configuration.");
ABSL_FLAG(kythe::PathCanonicalizer::Policy, canonicalize_vname_paths,
          kythe::PathCanonicalizer::Policy::kCleanOnly,
          "Policy to use when canonicalization VName paths: "
          "clean-only (default), prefer-relative, prefer-real.");

static void LoadExtraAction(const std::string& path,
                            blaze::ExtraActionInfo* info,
                            blaze::CppCompileInfo* cpp_info) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  CodedInputStream coded_input_stream(&file_input_stream);
  coded_input_stream.SetTotalBytesLimit(INT_MAX);
  CHECK(info->ParseFromCodedStream(&coded_input_stream));
  close(fd);
  CHECK(info->HasExtension(blaze::CppCompileInfo::cpp_compile_info));
  *cpp_info = info->GetExtension(blaze::CppCompileInfo::cpp_compile_info);
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  if (remain.size() != 4) {
    absl::FPrintF(stderr,
                  "Call as %s extra-action-file output-file vname-config\n",
                  remain[0]);
    return 1;
  }
  std::string extra_action_file = remain[1];
  std::string output_file = remain[2];
  std::string vname_config = remain[3];
  blaze::ExtraActionInfo info;
  blaze::CppCompileInfo cpp_info;
  LoadExtraAction(extra_action_file, &info, &cpp_info);

  const std::string& source = cpp_info.source_file();
  if (absl::EndsWith(source, ".s") || absl::EndsWith(source, ".asm")) {
    // Assembly files can be in the srcs of CppCompile actions (created with
    // cc_* rules or Starlark rules using cc_common.compile). However, these
    // actions don't actually run the compiler, so there's nothing to extract.
    std::ofstream output(output_file);
    return 0;
  }

  kythe::ExtractorConfiguration config;
  std::vector<std::string> args;
  args.push_back(cpp_info.tool());
  args.insert(args.end(), cpp_info.compiler_option().begin(),
              cpp_info.compiler_option().end());

  // If the command-line did not specify "-c x.cc" specifically, include the
  // primary source at the end of the argument list.
  if (std::find(args.begin(), args.end(), "-c") == args.end()) {
    args.push_back(cpp_info.source_file());
  }
  config.SetOutputFile(output_file);
  config.SetArgs(args);
  config.SetVNameConfig(vname_config);
  config.SetTargetName(info.owner());
  config.SetBuildConfig(absl::GetFlag(FLAGS_build_config));
  config.SetCompilationOutputPath(cpp_info.output_file());
  config.SetPathCanonizalizationPolicy(
      absl::GetFlag(FLAGS_canonicalize_vname_paths));
  config.Extract(kythe::supported_language::Language::kCpp);
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
