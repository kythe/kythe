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

#include <fcntl.h>

#include <fstream>
#include <functional>
#include <iostream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/indexing/KytheCachingOutput.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/indexer/textproto/analyzer.h"
#include "kythe/cxx/indexer/textproto/plugin_registry.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/buildinfo.pb.h"

ABSL_FLAG(std::string, o, "-", "Output filename.");
ABSL_FLAG(bool, flush_after_each_entry, true,
          "Flush output after writing each entry.");
ABSL_FLAG(std::string, index_file, "", "Path to a KZip file to index.");

namespace kythe {
namespace lang_textproto {
namespace {
/// Callback function to process a single compilation unit.
using CompilationVisitCallback = std::function<void(
    const proto::CompilationUnit&, std::vector<proto::FileData> file_data)>;

/// \brief Reads all compilations from a .kzip file into memory.
/// \param path The path from which the file should be read.
/// \param visit Callback function called for each compiliation unit within
/// the kzip.
// TODO(justbuchanan): Refactor so that this function is shared with the cxx
// indexer. It was initially copied from cxx/indexer/frontend.cc.
void DecodeKzipFile(const std::string& path,
                    const CompilationVisitCallback& visit) {
  // This forces the BuildDetails proto descriptor to be added to the pool so
  // we can deserialize it.
  proto::BuildDetails needed_for_proto_deserialization;

  absl::StatusOr<IndexReader> reader = kythe::KzipReader::Open(path);
  CHECK(reader.ok()) << "Couldn't open kzip from " << path << ": "
                     << reader.status();
  bool compilation_read = false;
  auto status = reader->Scan([&](absl::string_view digest) {
    std::vector<proto::FileData> virtual_files;
    auto compilation = reader->ReadUnit(digest);
    CHECK(compilation.ok()) << compilation.status();
    for (const auto& file : compilation->unit().required_input()) {
      auto content = reader->ReadFile(file.info().digest());
      CHECK(content.ok()) << "Unable to read file with digest: "
                          << file.info().digest() << ": " << content.status();
      proto::FileData file_data;
      file_data.set_content(*content);
      file_data.mutable_info()->set_path(file.info().path());
      file_data.mutable_info()->set_digest(file.info().digest());
      virtual_files.push_back(std::move(file_data));
    }

    visit(compilation->unit(), std::move(virtual_files));

    compilation_read = true;
    return true;
  });
  CHECK(status.ok()) << status;
  CHECK(compilation_read) << "Missing compilation in " << path;
}

int main(int argc, char* argv[]) {
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(
      R"(Command-line frontend for the Kythe Textproto indexer.

Example:
  indexer -o foo.bin --index_file foo.kzip")");
  absl::ParseCommandLine(argc, argv);

  CHECK(!absl::GetFlag(FLAGS_index_file).empty())
      << "Please provide a kzip file path to --index_file.";

  std::string output = absl::GetFlag(FLAGS_o);
  int write_fd = STDOUT_FILENO;
  if (output != "-") {
    // TODO: do we need all these flags?
    write_fd = ::open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    CHECK(write_fd != -1) << "Can't open output file";
  }

  google::protobuf::io::FileOutputStream raw_output(write_fd);
  raw_output.SetCloseOnDelete(true);
  FileOutputStream kythe_output(&raw_output);
  kythe_output.set_flush_after_each_entry(
      absl::GetFlag(FLAGS_flush_after_each_entry));
  KytheGraphRecorder recorder(&kythe_output);

  DecodeKzipFile(absl::GetFlag(FLAGS_index_file),
                 [&](const proto::CompilationUnit& unit,
                     std::vector<proto::FileData> file_data) {
                   absl::Status status = lang_textproto::AnalyzeCompilationUnit(
                       kythe::lang_textproto::LoadRegisteredPlugins, unit,
                       file_data, &recorder);
                   CHECK(status.ok()) << status;
                 });

  return 0;
}

}  // namespace
}  // namespace lang_textproto
}  // namespace kythe

int main(int argc, char* argv[]) { kythe::lang_textproto::main(argc, argv); }
