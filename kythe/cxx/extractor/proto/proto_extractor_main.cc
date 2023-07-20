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

// Standalone extractor for protobuf. Given a proto file, builds a kzip
// containing the proto file(s) and all dependencies.
//
// Usage:
//   export KYTHE_OUTPUT_FILE=foo.kzip
//   proto_extractor foo.proto
//   proto_extractor foo.proto bar.proto
//   proto_extractor foo.proto -- --proto_path dir/with/my/deps

#include <string>

#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/match.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/kzip_writer.h"
#include "kythe/cxx/extractor/proto/proto_extractor.h"
#include "kythe/cxx/indexer/proto/search_path.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace lang_proto {
namespace {
/// \brief Returns a kzip-based IndexWriter or dies.
IndexWriter OpenKzipWriterOrDie(absl::string_view path) {
  auto writer = KzipWriter::Create(path);
  CHECK(writer.ok()) << "Failed to open KzipWriter: " << writer.status();
  return std::move(*writer);
}
}  // namespace

int main(int argc, char* argv[]) {
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(
      R"(Standalone extractor for the Kythe Proto indexer.
Creates a Kzip containing the main proto file(s) and any dependencies.

Examples:
  export KYTHE_OUTPUT_FILE=foo.kzip
  proto_extractor foo.proto
  proto_extractor foo.proto bar.proto
  proto_extractor foo.proto -- --proto_path dir/with/my/deps")");
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  std::vector<std::string> final_args(remain.begin() + 1, remain.end());

  ProtoExtractor extractor;
  extractor.ConfigureFromEnv();

  const char* env_output_file = getenv("KYTHE_OUTPUT_FILE");
  CHECK(env_output_file != nullptr)
      << "Please specify an output kzip file with the KYTHE_OUTPUT_FILE "
         "environment variable.";
  IndexWriter kzip_writer = OpenKzipWriterOrDie(env_output_file);

  // Parse --proto_path and -I args into a set of path substitutions (search
  // paths). The remaining arguments should be .proto files.
  std::vector<std::string> proto_filenames;
  ::kythe::lang_proto::ParsePathSubstitutions(
      final_args, &extractor.path_substitutions, &proto_filenames);
  for (const std::string& arg : proto_filenames) {
    CHECK(absl::EndsWith(arg, ".proto"))
        << "Invalid arg, expected a proto file: '" << arg << "'";
  }
  CHECK(!proto_filenames.empty()) << "Expected 1+ .proto files.";

  // Extract and save kzip.
  proto::IndexedCompilation compilation;
  *compilation.mutable_unit() =
      extractor.ExtractProtos(proto_filenames, &kzip_writer);
  auto digest = kzip_writer.WriteUnit(compilation);
  CHECK(digest.ok()) << "Error writing unit to kzip: " << digest.status();
  CHECK(kzip_writer.Close().ok());

  return 0;
}

}  // namespace lang_proto
}  // namespace kythe

int main(int argc, char* argv[]) { return kythe::lang_proto::main(argc, argv); }
