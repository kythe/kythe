/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// objc_extractor_bazel is a Objective-C extractor meant to be run as a Bazel
// extra_action. It may be used with third_party/bazel/get_devdir.sh and
// third_party/bazel/get_sdkroot.sh to fix placeholders left in arguments by
// Bazel.
//
// For example:
//
//  action_listener(
//    name = "extract_kzip",
//    extra_actions = [":extra_action"],
//    mnemonics = ["ObjcCompile"],
//    visibility = ["//visibility:public"],
//  )
//
//  extra_action(
//    name = "extra_action",
//    cmd = "$(location :objc_extractor_binary) \
//             $(EXTRA_ACTION_FILE) \
//             $(output $(ACTION_ID).objc.kzip) \
//             $(location :vnames_config) \
//             $(location :get_devdir) \
//             $(location :get_sdkroot)",
//    data = [
//      ":get_devdir",
//      ":get_sdkroot",
//      ":vnames_config",
//    ],
//    out_templates = ["$(ACTION_ID).objc.kzip"],
//    tools = [":objc_extractor_binary"],
//  )
//
//  # In this example, the extractor binary is pre-built.
//  filegroup(
//    name = "objc_extractor_binary",
//    srcs = ["objc_extractor_bazel"],
//  )
//
//  filegroup(
//    name = "vnames_config",
//    srcs = ["vnames.json"],
//  )
//
//  sh_binary(
//    name = "get_devdir",
//    srcs = ["get_devdir.sh"],
//  )
//
//  sh_binary(
//    name = "get_sdkroot",
//    srcs = ["get_sdkroot.sh"],
//  )

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "cxx_extractor.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/extractor/language.h"
#include "objc_bazel_support.h"
#include "third_party/bazel/src/main/protobuf/extra_actions_base.pb.h"

ABSL_FLAG(kythe::PathCanonicalizer::Policy, canonicalize_vname_paths,
          kythe::PathCanonicalizer::Policy::kCleanOnly,
          "Policy to use when canonicalization VName paths: "
          "clean-only (default), prefer-relative, prefer-real.");

struct XAState {
  std::string extra_action_file;
  std::string output_file;
  std::string vname_config;
  std::string devdir_script;
  std::string sdkroot_script;
};

static bool ContainsUnsupportedArg(const std::vector<std::string>& args) {
  for (const auto& arg : args) {
    // We do not support compilations using modules yet.
    if (arg == "-fmodules") {
      return true;
    }
  }
  return false;
}

static bool LoadSpawnInfo(const XAState& xa_state,
                          const blaze::ExtraActionInfo& info,
                          kythe::ExtractorConfiguration& config) {
  blaze::SpawnInfo spawn_info = info.GetExtension(blaze::SpawnInfo::spawn_info);

  std::vector<std::string> args;
  // If the user didn't specify a script path, don't mutate the arguments in the
  // extra action.
  if (xa_state.devdir_script.empty() || xa_state.sdkroot_script.empty()) {
    for (const auto& i : spawn_info.argument()) {
      std::string arg = i;
      args.push_back(arg);
    }
  } else {
    auto cmdPrefix = kythe::BuildEnvVarCommandPrefix(spawn_info.variable());
    auto devdir = kythe::RunScript(cmdPrefix + xa_state.devdir_script);
    auto sdkroot = kythe::RunScript(cmdPrefix + xa_state.sdkroot_script);

    kythe::FillWithFixedArgs(args, spawn_info, devdir, sdkroot);
  }

  if (ContainsUnsupportedArg(args)) {
    LOG(ERROR) << "Not extracting " << info.owner()
               << " because it had an unsupported argument.";
    return false;
  }

  config.SetOutputFile(xa_state.output_file);
  config.SetArgs(args);
  config.SetVNameConfig(xa_state.vname_config);
  config.SetTargetName(info.owner());
  if (spawn_info.output_file_size() > 0) {
    config.SetCompilationOutputPath(spawn_info.output_file(0));
  }
  return true;
}

static bool LoadCppInfo(const XAState& xa_state,
                        const blaze::ExtraActionInfo& info,
                        kythe::ExtractorConfiguration& config) {
  blaze::CppCompileInfo cpp_info =
      info.GetExtension(blaze::CppCompileInfo::cpp_compile_info);

  std::vector<std::string> args;
  // If the user didn't specify a script path, don't mutate the arguments in the
  // extra action.
  if (xa_state.devdir_script.empty() || xa_state.sdkroot_script.empty()) {
    args.push_back(cpp_info.tool());
    for (const auto& i : cpp_info.compiler_option()) {
      std::string arg = i;
      args.push_back(arg);
    }
  } else {
    auto cmdPrefix = kythe::BuildEnvVarCommandPrefix(cpp_info.variable());
    auto devdir = kythe::RunScript(cmdPrefix + xa_state.devdir_script);
    auto sdkroot = kythe::RunScript(cmdPrefix + xa_state.sdkroot_script);

    kythe::FillWithFixedArgs(args, cpp_info, devdir, sdkroot);
  }

  if (ContainsUnsupportedArg(args)) {
    LOG(ERROR) << "Not extracting " << info.owner()
               << " because it had an unsupported argument.";
    return false;
  }

  config.SetOutputFile(xa_state.output_file);
  config.SetArgs(args);
  config.SetVNameConfig(xa_state.vname_config);
  config.SetTargetName(info.owner());
  config.SetCompilationOutputPath(cpp_info.output_file());
  return true;
}

static bool LoadExtraAction(const XAState& xa_state,
                            kythe::ExtractorConfiguration& config) {
  using namespace google::protobuf::io;
  blaze::ExtraActionInfo info;
  int fd =
      open(xa_state.extra_action_file.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << xa_state.extra_action_file;
  FileInputStream file_input_stream(fd);
  CodedInputStream coded_input_stream(&file_input_stream);
  coded_input_stream.SetTotalBytesLimit(INT_MAX);
  CHECK(info.ParseFromCodedStream(&coded_input_stream));
  close(fd);

  if (info.HasExtension(blaze::SpawnInfo::spawn_info)) {
    return LoadSpawnInfo(xa_state, info, config);
  } else if (info.HasExtension(blaze::CppCompileInfo::cpp_compile_info)) {
    return LoadCppInfo(xa_state, info, config);
  }
  LOG(ERROR)
      << "ObjcCompile Extra Action didn't have SpawnInfo or CppCompileInfo.";
  return false;
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  if (remain.size() != 4 && remain.size() != 6) {
    absl::FPrintF(
        stderr,
        "Invalid number of arguments:\n\tCall as %s extra-action-file "
        "output-file vname-config [devdir-script sdkroot-script]\n",
        remain[0]);
    return 1;
  }
  XAState xa_state;
  xa_state.extra_action_file = remain[1];
  xa_state.output_file = remain[2];
  xa_state.vname_config = remain[3];
  if (remain.size() == 6) {
    xa_state.devdir_script = remain[4];
    xa_state.sdkroot_script = remain[5];
  } else {
    xa_state.devdir_script = "";
    xa_state.sdkroot_script = "";
  }

  kythe::ExtractorConfiguration config;
  config.SetPathCanonizalizationPolicy(
      absl::GetFlag(FLAGS_canonicalize_vname_paths));
  bool success = LoadExtraAction(xa_state, config);
  if (success) {
    config.Extract(kythe::supported_language::Language::kObjectiveC);
  } else {
    LOG(ERROR) << "Couldn't load extra action";
    // If we couldn't extract, just write an empty output file. This way the
    // extra_action will be a success from bazel's perspective, which should
    // remove some log spam.
    auto F = fopen(xa_state.output_file.c_str(), "w");
    if (F != nullptr) {
      fclose(F);
    }
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
