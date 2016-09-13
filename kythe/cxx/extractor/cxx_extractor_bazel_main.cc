/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "third_party/bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/stubs/common.h"

#include "cxx_extractor.h"

static void LoadExtraAction(const std::string &path,
                            blaze::ExtraActionInfo *info,
                            blaze::CppCompileInfo *cpp_info) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  CodedInputStream coded_input_stream(&file_input_stream);
  coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
  CHECK(info->ParseFromCodedStream(&coded_input_stream));
  close(fd);
  CHECK(info->HasExtension(blaze::CppCompileInfo::cpp_compile_info));
  *cpp_info = info->GetExtension(blaze::CppCompileInfo::cpp_compile_info);
}

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetVersionString("0.1");
  if (argc != 4) {
    fprintf(stderr, "Call as %s extra-action-file output-file vname-config\n",
            argv[0]);
    return 1;
  }
  std::string extra_action_file = argv[1];
  std::string output_file = argv[2];
  std::string vname_config = argv[3];
  blaze::ExtraActionInfo info;
  blaze::CppCompileInfo cpp_info;
  LoadExtraAction(extra_action_file, &info, &cpp_info);
  kythe::ExtractorConfiguration config;
  std::vector<std::string> args;
  args.push_back(cpp_info.tool());
  args.insert(args.end(), cpp_info.compiler_option().begin(),
              cpp_info.compiler_option().end());
  args.push_back(cpp_info.source_file());
  config.SetKindexOutputFile(output_file);
  config.SetArgs(args);
  config.SetVNameConfig(vname_config);
  config.Extract();
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
