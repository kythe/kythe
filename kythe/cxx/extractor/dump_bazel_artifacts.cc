/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/extractor/bazel_artifact_reader.h"

ABSL_FLAG(std::string, build_event_binary_file, "",
          "Bazel event protocol file to read");

namespace kythe {
namespace {

absl::string_view Basename(absl::string_view path) {
  if (auto pos = path.rfind('/'); pos != path.npos) {
    return path.substr(pos + 1);
  }
  return path;
}

int DumpArtifacts(const std::string filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    LOG(ERROR) << "Error opening " << filename << ": " << std::strerror(errno);
    return 1;
  }

  google::protobuf::io::IstreamInputStream input(&file);
  BazelEventReader events(&input);
  BazelArtifactReader artifacts(&events);
  for (; !artifacts.Done(); artifacts.Next()) {
    std::cout << artifacts.Ref().label << std::endl;
    for (const auto& [local_path, uri] : artifacts.Ref().files) {
      std::cout << "  " << Basename(uri) << std::endl;
    }
  }
  if (!artifacts.status().ok()) {
    LOG(ERROR) << artifacts.status();
  }

  return !artifacts.status().ok();
}
}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  kythe::InitializeProgram(argv[0]);
  absl::ParseCommandLine(argc, argv);
  return kythe::DumpArtifacts(absl::GetFlag(FLAGS_build_event_binary_file));
}
