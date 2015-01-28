/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Allows the Kythe C++ indexer to be invoked from the command line. By default,
// this program reads a single C++ compilation unit from stdin and emits
// binary Kythe artifacts to stdout as a sequence of Entity protos.
// Command-line arguments may be passed to Clang as positional parameters.
//
//   eg: indexer -i foo.cc -o foo.bin -- -DINDEXING
//       indexer -i foo.cc | verifier foo.cc
//       indexer some/index.kindex

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/index_pack.h"
#include "kythe/proto/analysis.pb.h"

#include "IndexerFrontendAction.h"
#include "KytheGraphObserver.h"
#include "KytheGraphRecorder.h"
#include "KytheOutputStream.h"

DEFINE_string(o, "-", "Output filename");
DEFINE_string(i, "-", "Input filename");
DEFINE_bool(ignore_unimplemented, false,
            "Continue indexing even if we find something we don't support.");
DEFINE_bool(flush_after_each_entry, false,
            "Flush output after writing each entry.");
DEFINE_bool(index_template_instantiations, true,
            "Index template instantiations.");
DEFINE_string(index_pack, "", "Mount an index pack rooted at this directory.");

namespace kythe {

/// \brief Reads data from a .kindex file into memory.
/// \param path The path from which the file should be read.
/// \param virtual_files A vector to be filled with FileData.
/// \param unit A `CompilationUnit` to be decoded from the .kindex.
static void DecodeIndexFile(const std::string &path,
                            std::vector<proto::FileData> *virtual_files,
                            proto::CompilationUnit *unit) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  CodedInputStream coded_input_stream(&gzip_input_stream);
  google::protobuf::uint32 byte_size;
  while (coded_input_stream.ReadVarint32(&byte_size)) {
    auto limit = coded_input_stream.PushLimit(byte_size);
    if (unit) {
      CHECK(unit->ParseFromCodedStream(&coded_input_stream));
      unit = nullptr;
    } else {
      proto::FileData content;
      CHECK(content.ParseFromCodedStream(&coded_input_stream));
      CHECK(content.has_info());
      virtual_files->push_back(std::move(content));
    }
    coded_input_stream.PopLimit(limit);
  }
  CHECK(!unit) << "Never saw a CompilationUnit.";
  close(fd);
}

static void DecodeIndexPack(const std::string &cu_hash,
                            std::unique_ptr<IndexPack> index_pack,
                            std::vector<proto::FileData> *virtual_files,
                            proto::CompilationUnit *unit) {
  std::string error_text;
  CHECK(index_pack->ReadCompilationUnit(cu_hash, unit, &error_text))
      << "Could not read " << cu_hash << ": " << error_text;
  for (const auto &input : unit->required_input()) {
    const auto &info = input.info();
    CHECK(!info.path().empty());
    CHECK(!info.digest().empty())
        << "Required input " << info.path() << " is missing its digest.";
    std::string read_data;
    CHECK(index_pack->ReadFileData(info.digest(), &read_data))
        << "Could not read " << info.path() << " (digest " << info.digest()
        << ") from the index pack: " << read_data;
    proto::FileData file_data;
    file_data.set_content(read_data);
    file_data.mutable_info()->set_path(info.path());
    file_data.mutable_info()->set_digest(info.digest());
    virtual_files->push_back(std::move(file_data));
  }
}

/// \brief Does `input` end with `suffix`?
static bool EndsWith(const std::string &input, const std::string &suffix) {
  return input.size() >= suffix.size() &&
         !input.compare(input.size() - suffix.size(), suffix.size(), suffix);
}

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::SetVersionString("0.1");
  google::SetUsageMessage(R"(Command-line frontend for the Kythe C++ indexer.
Invokes the Kythe C++ indexer on a single compilation unit. By default reads
source text from stdin and writes binary Kythe artifacts to stdout as a sequence
of Entity protos. Command-line arguments may be passed to Clang as positional
parameters.

If -index_pack is not specified, there may be a positional parameter specified
that ends in .kindex. If one exists, no other positional parameters may be
specified, nor may an additional input parameter be specified. Input will
be read from the index file.

If -index_pack is specified, there must be exactly one positional parameter.
This parameter should be the compilation unit ID from the mounted index pack
that is meant to be indexed. No additional input parameters may be specified.

Examples:
  indexer -index_pack path/to/pack/root 660f1f840000000000
  indexer some/index.kindex
  indexer -i foo.cc -o foo.bin -- -DINDEXING
  indexer -i foo.cc | verifier foo.cc
  indexer -i foo.cc | gqui from rawproto:- proto \
      storage.proto:kythe.proto.Entry")");
  google::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<std::string> final_args(argv, argv + argc);

  // Check to see if we should be using an index pack or a .kindex file.
  std::unique_ptr<kythe::IndexPack> index_pack;
  std::string kindex_file_or_cu;
  if (!FLAGS_index_pack.empty()) {
    std::string error_text;
    auto filesystem = kythe::IndexPackPosixFilesystem::Open(
        FLAGS_index_pack, kythe::IndexPackFilesystem::OpenMode::kReadOnly,
        &error_text);
    CHECK(filesystem) << "Couldn't open index pack from " << FLAGS_index_pack
                      << ": " << error_text;
    index_pack.reset(new kythe::IndexPack(std::move(filesystem)));
    CHECK(final_args.size() >= 2) << "You must specify a compilation unit.";
    kindex_file_or_cu = final_args[1];
  } else {
    std::string kindex_suffix = ".kindex";
    for (const auto &arg : final_args) {
      if (EndsWith(arg, kindex_suffix)) {
        kindex_file_or_cu = arg;
        break;
      }
    }
  }

  if (!kindex_file_or_cu.empty()) {
    CHECK_EQ(2, final_args.size())
        << "No other positional arguments are allowed when reading "
        << "from an index file or an index pack.";
    CHECK_EQ("-", FLAGS_i)
        << "No other input is allowed when reading from an index file or an "
        << "index pack.";
  }

  std::vector<proto::FileData> virtual_files;
  clang::FileSystemOptions file_system_options;
  proto::CompilationUnit unit;

  if (!kindex_file_or_cu.empty()) {
    if (index_pack) {
      DecodeIndexPack(kindex_file_or_cu, std::move(index_pack), &virtual_files,
                      &unit);
    } else {
      DecodeIndexFile(kindex_file_or_cu, &virtual_files, &unit);
    }
    // CompilationUnit's arguments field includes the names of source files.
    final_args.assign(unit.argument().begin(), unit.argument().end());
    // We presently handle kindex files with only one main source file.
    CHECK_EQ(1, unit.source_file_size());
    file_system_options.WorkingDir = unit.working_directory();
  } else {
    int read_fd = STDIN_FILENO;
    std::string source_file_name = "stdin.cc";

    if (FLAGS_i != "-") {
      read_fd = open(FLAGS_i.c_str(), O_RDONLY);
      if (read_fd == -1) {
        perror("Can't open input file");
        exit(1);
      }
      source_file_name = FLAGS_i;
    }

    final_args.push_back(source_file_name);

    char buf[1024];
    llvm::SmallString<1024> source_data;
    ssize_t amount_read;
    while ((amount_read = read(read_fd, buf, 1024)) > 0) {
      source_data.append(llvm::StringRef(buf, amount_read));
    }
    if (amount_read < 0) {
      perror("Error reading input file");
      exit(1);
    }
    close(read_fd);
    // clang wants the source file to be null-terminated, but this should
    // not be in range of the StringRef. std::string ends with \0.
    proto::FileData file_data;
    file_data.mutable_info()->set_path(source_file_name);
    file_data.set_content(source_data.str());
    virtual_files.push_back(std::move(file_data));
  }

  int write_fd = STDOUT_FILENO;
  if (FLAGS_o != "-") {
    write_fd = open(FLAGS_o.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (write_fd == -1) {
      perror("Can't open output file");
      exit(1);
    }
  }

  bool had_no_errors;
  {
    google::protobuf::io::FileOutputStream raw_output(write_fd);
    FileOutputStream kythe_output(&raw_output);
    KytheGraphRecorder kythe_recorder(&kythe_output);
    KytheGraphObserver observer(&kythe_recorder);
    std::map<std::string, proto::VName> path_to_vname;
    for (const auto &input : unit.required_input()) {
      if (input.has_info() && input.info().has_path() && input.has_v_name()) {
        observer.set_path_vname(input.info().path(), input.v_name());
        path_to_vname[input.info().path()] = input.v_name();
      }
    }
    std::unique_ptr<IndexerFrontendAction> action(
        new IndexerFrontendAction(&observer));
    action->setIgnoreUnimplemented(FLAGS_ignore_unimplemented
                                       ? BehaviorOnUnimplemented::Continue
                                       : BehaviorOnUnimplemented::Abort);
    action->setTemplateMode(FLAGS_index_template_instantiations
                                ? BehaviorOnTemplates::VisitInstantiations
                                : BehaviorOnTemplates::SkipInstantiations);
    llvm::IntrusiveRefCntPtr<clang::FileManager> file_manager(
        new clang::FileManager(file_system_options));
    final_args.insert(final_args.begin() + 1, "-fsyntax-only");
    clang::tooling::ToolInvocation invocation(final_args, action.release(),
                                              file_manager.get());
    // It seems as though Clang doesn't use file_system_options.WorkingDir
    // as expected. Make another mapping with relative paths when we need it.
    std::string expect_prefix;
    if (!file_system_options.WorkingDir.empty()) {
      expect_prefix = (file_system_options.WorkingDir +
                       llvm::sys::path::get_separator()).str();
    }
    for (const auto &file : virtual_files) {
      llvm::StringRef file_content(file.content().data(),
                                   file.content().size());
      const std::string &file_path = file.info().path();
      invocation.mapVirtualFile(file_path, file_content);
      if (!expect_prefix.empty() &&
          llvm::StringRef(file_path).startswith(expect_prefix)) {
        std::string relative_path = file_path.substr(expect_prefix.size());
        invocation.mapVirtualFile(relative_path, file_content);
        const auto vname_iterator = path_to_vname.find(file_path);
        if (vname_iterator != path_to_vname.end()) {
          observer.set_path_vname(relative_path, vname_iterator->second);
        }
      }
    }
    had_no_errors = invocation.run();
  }

  if (close(write_fd) != 0) {
    perror("Error closing output file");
    exit(1);
  }

  return had_no_errors == false;
}

}  // namespace kythe

int main(int argc, char *argv[]) { return kythe::main(argc, argv); }
