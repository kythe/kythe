/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Allows the Kythe Proto indexer to be invoked from the command line. By
// default, this program reads a single Proto compilation unit from stdin and
// emits binary Kythe artifacts to stdout as a sequence of Entity protos.
//
//   eg: indexer foo.proto -o foo.bin
//       indexer foo.proto | verifier foo.proto
//       indexer -index_file some/file.kzip
//       cat foo.proto | indexer | verifier foo.proto

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheCachingOutput.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/indexer/proto/indexer_frontend.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/buildinfo.pb.h"

ABSL_FLAG(std::string, o, "-", "Output filename.");
ABSL_FLAG(bool, flush_after_each_entry, false,
          "Flush output after writing each entry.");
ABSL_FLAG(std::string, index_file, "",
          ".kzip file containing compilation unit.");
ABSL_FLAG(std::string, default_corpus, "", "Default corpus for VNames.");
ABSL_FLAG(std::string, default_root, "", "Default root for VNames.");

namespace kythe {
namespace {

/// Callback function to process a single compilation unit.
using CompilationVisitCallback = std::function<void(
    const proto::CompilationUnit&, std::vector<proto::FileData> file_data)>;

/// \brief Reads all compilations from a .kzip file into memory.
/// \param path The path from which the file should be read.
/// \param visit Callback function called for each compiliation unit within the
/// kzip.
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
  CHECK(status.ok()) << status.ToString();
  CHECK(compilation_read) << "Missing compilation in " << path;
}

bool ReadProtoFile(int fd, const std::string& relative_path,
                   const proto::VName& file_vname,
                   std::vector<proto::FileData>* files,
                   proto::CompilationUnit* unit) {
  char buf[1024];
  std::string source_data;
  ssize_t amount_read;
  while ((amount_read = ::read(fd, buf, sizeof buf)) > 0) {
    absl::StrAppend(&source_data, absl::string_view(buf, amount_read));
  }
  if (amount_read < 0) {
    LOG(ERROR) << "Error reading input file";
    return false;
  }
  proto::FileData file_data;
  file_data.set_content(source_data);
  file_data.mutable_info()->set_path(CleanPath(relative_path));
  proto::CompilationUnit::FileInput* file_input = unit->add_required_input();
  *file_input->mutable_v_name() = file_vname;
  *file_input->mutable_info() = file_data.info();
  // keep the filename as entered on the command line in the argument list
  unit->add_argument(relative_path);
  unit->add_source_file(file_data.info().path());
  files->push_back(std::move(file_data));
  return true;
}

}  // anonymous namespace

int main(int argc, char* argv[]) {
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(
      R"(Command-line frontend for the Kythe Proto indexer.
Invokes the Kythe Proto indexer on compilation unit(s). By default writes binary
Kythe artifacts to STDOUT as a sequence of Entity protos; this destination can
be overridden with the argument of -o.

If -index_file is specified, input will be read from its argument (which will
typically end in .kzip). No other positional parameters may be specified, nor
may an additional input parameter be specified.

If -index_file is not specified, all positional parameters (and any flags
following "--") are taken as arguments to the Proto compiler. Those ending in
.proto are taken as the filenames of the compilation unit to be analyzed; all
arguments are added to the compilation unit's "arguments" field. If no .proto
filenames are among these arguments, or if "-" is supplied, then source text
will be read from STDIN (and named "stdin.proto").

Examples:
  indexer -index_file index.kzip
  indexer -o foo.bin -- -Isome/path -Isome/other/path foo.proto
  indexer foo.proto bar.proto | verifier foo.proto bar.proto")");
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  std::vector<std::string> final_args(remain.begin() + 1, remain.end());

  std::string kzip_file;
  if (!absl::GetFlag(FLAGS_index_file).empty()) {
    CHECK(final_args.empty())
        << "No positional arguments are allowed when reading "
        << "from an index file.";
    kzip_file = absl::GetFlag(FLAGS_index_file);
  }

  int write_fd = STDOUT_FILENO;
  if (absl::GetFlag(FLAGS_o) != "-") {
    write_fd =
        ::open(absl::GetFlag(FLAGS_o).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    CHECK(write_fd != -1) << "Can't open output file";
  }

  bool had_error = false;

  {
    google::protobuf::io::FileOutputStream raw_output(write_fd);
    kythe::FileOutputStream kythe_output(&raw_output);
    kythe_output.set_flush_after_each_entry(
        absl::GetFlag(FLAGS_flush_after_each_entry));

    if (!kzip_file.empty()) {
      DecodeKzipFile(kzip_file, [&](const proto::CompilationUnit& unit,
                                    std::vector<proto::FileData> file_data) {
        std::string err =
            IndexProtoCompilationUnit(unit, file_data, &kythe_output);
        if (!err.empty()) {
          had_error = true;
          LOG(ERROR) << "Error: " << err;
        }
      });
    } else {
      std::vector<proto::FileData> files;
      proto::CompilationUnit unit;

      unit.set_working_directory(GetCurrentDirectory().value());
      FileVNameGenerator file_vnames;
      kythe::proto::VName default_vname;
      default_vname.set_corpus(absl::GetFlag(FLAGS_default_corpus));
      default_vname.set_root(absl::GetFlag(FLAGS_default_root));
      file_vnames.set_default_base_vname(default_vname);
      bool stdin_requested = false;
      for (const std::string& arg : final_args) {
        if (arg == "-") {
          stdin_requested = true;
        } else if (absl::EndsWith(arg, ".proto")) {
          int read_fd = ::open(arg.c_str(), O_RDONLY);
          proto::VName file_vname = file_vnames.LookupVName(arg);
          file_vname.set_path(CleanPath(file_vname.path()));
          CHECK(ReadProtoFile(read_fd, arg, file_vname, &files, &unit))
              << "Read error for " << arg;
          ::close(read_fd);
        } else {
          LOG(ERROR) << "Adding protoc argument: " << arg;
          unit.add_argument(arg);
        }
      }
      if (stdin_requested || files.empty()) {
        proto::VName stdin_vname;
        stdin_vname.set_corpus(unit.v_name().corpus());
        stdin_vname.set_root(unit.v_name().root());
        stdin_vname.set_path("stdin.proto");
        CHECK(ReadProtoFile(STDIN_FILENO, "stdin.proto", stdin_vname, &files,
                            &unit))
            << "Read error for protobuf on STDIN";
      }

      std::string err = IndexProtoCompilationUnit(unit, files, &kythe_output);
      if (!err.empty()) {
        had_error = true;
        LOG(ERROR) << "Error: " << err;
      }
    }
  }

  CHECK(::close(write_fd) == 0) << "Error closing output file";

  return had_error ? 1 : 0;
}

}  // namespace kythe

int main(int argc, char* argv[]) { return kythe::main(argc, argv); }
