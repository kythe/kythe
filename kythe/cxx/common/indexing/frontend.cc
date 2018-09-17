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

#include "kythe/cxx/common/indexing/frontend.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include "absl/memory/memory.h"
#include "gflags/gflags.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/proto_conversions.h"
#include "kythe/proto/claim.pb.h"
#include "llvm/ADT/STLExtras.h"

DEFINE_string(o, "-", "Output filename");
DEFINE_string(i, "-", "Input filename");
DEFINE_bool(ignore_unimplemented, true,
            "Continue indexing even if we find something we don't support.");
DEFINE_bool(flush_after_each_entry, true,
            "Flush output after writing each entry.");
DEFINE_string(static_claim, "", "Use a static claim table.");
DEFINE_bool(claim_unknown, true, "Process files with unknown claim status.");
DEFINE_string(index_pack, "", "Mount an index pack rooted at this directory.");
DEFINE_string(cache, "", "Use a memcache instance (ex: \"--SERVER=foo:1234\")");
DEFINE_int32(min_size, 4096, "Minimum size of an entry bundle");
DEFINE_int32(max_size, 1024 * 32, "Maximum size of an entry bundle");
DEFINE_bool(cache_stats, false, "Show cache stats");
DEFINE_string(icorpus, "", "Corpus to use for files specified with -i");
DEFINE_bool(normalize_file_vnames, false, "Normalize incoming file vnames.");
DEFINE_string(experimental_dynamic_claim_cache, "",
              "Use a memcache instance for dynamic claims (EXPERIMENTAL)");
// Setting this to a value > 1 allows the same object (e.g., a transcript of
// an include file) to be claimed multiple times. In the absence of transcript
// labels, setting this value to 1 means that only one environment will be
// considered when indexing a vname. This may result in (among other effects)
// conditionally included code never being indexed if the symbols checked differ
// between translation units.
DEFINE_uint64(experimental_dynamic_overclaim, 1,
              "Maximum number of dynamic claims per claimable (EXPERIMENTAL)");
DEFINE_bool(test_claim, false, "Use an in-memory claim database for testing.");

namespace kythe {

namespace {
/// The prefix prepended to silent inputs. Only checked when "--test_claim"
/// is enabled.
constexpr char kSilentPrefix[] = "silent:";
/// \return the input name stripped of its prefix if it's silent; an empty
/// string otherwise.
llvm::StringRef strip_silent_input_prefix(llvm::StringRef argument) {
  if (FLAGS_test_claim && argument.startswith(kSilentPrefix)) {
    return argument.drop_front(::strlen(kSilentPrefix));
  }
  return {};
}
/// \brief Reads the output of the static claim tool.
///
/// `path` should be a file that contains a GZip-compressed sequence of
/// varint-prefixed wire format ClaimAssignment protobuf messages.
void DecodeStaticClaimTable(const std::string& path,
                            kythe::StaticClaimClient* client) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  google::protobuf::uint32 byte_size;
  for (;;) {
    CodedInputStream coded_input_stream(&gzip_input_stream);
    coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
    if (!coded_input_stream.ReadVarint32(&byte_size)) {
      break;
    }
    coded_input_stream.PushLimit(byte_size);
    kythe::proto::ClaimAssignment claim;
    CHECK(claim.ParseFromCodedStream(&coded_input_stream));
    // NB: We don't filter on compilation unit here. A dependency has three
    // static states (wrt some CU): unknown, owned by CU, owned by another CU.
    client->AssignClaim(claim.dependency_v_name(), claim.compilation_v_name());
  }
  close(fd);
}

/// \brief Reads data from a .kindex file into memory.
/// \param path The path from which the file should be read.
/// \param virtual_files A vector to be filled with FileData.
/// \param unit A `CompilationUnit` to be decoded from the .kindex.
void DecodeIndexFile(const std::string& path,
                     std::vector<proto::FileData>* virtual_files,
                     proto::CompilationUnit* unit) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  google::protobuf::uint32 byte_size;
  for (;;) {
    CodedInputStream coded_input_stream(&gzip_input_stream);
    coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
    if (!coded_input_stream.ReadVarint32(&byte_size)) {
      break;
    }
    coded_input_stream.PushLimit(byte_size);
    if (unit) {
      CHECK(unit->ParseFromCodedStream(&coded_input_stream));
      unit = nullptr;
    } else {
      proto::FileData content;
      CHECK(content.ParseFromCodedStream(&coded_input_stream));
      CHECK(content.has_info());
      virtual_files->push_back(std::move(content));
    }
  }
  CHECK(!unit) << "Never saw a CompilationUnit.";
  close(fd);
}

/// \brief Reads data from an index pack into memory.
/// \param cu_hash The hash of the compilation unit to read.
/// \param index_pack The index pack from which to read.
/// \param virtual_files A vector to be filled with FileData.
/// \param unit A `CompilationUnit` to be decoded from the index pack.
void DecodeIndexPack(const std::string& cu_hash,
                     std::unique_ptr<IndexPack> index_pack,
                     std::vector<proto::FileData>* virtual_files,
                     proto::CompilationUnit* unit) {
  std::string error_text;
  CHECK(index_pack->ReadCompilationUnit(cu_hash, unit, &error_text))
      << "Could not read " << cu_hash << ": " << error_text;
  for (const auto& input : unit->required_input()) {
    const auto& info = input.info();
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
}  // anonymous namespace

std::string IndexerContext::UsageMessage(const std::string& program_title,
                                         const std::string& program_name) {
  std::string message = "Command-line frontend for " + program_title;
  message.append(R"(.
Invokes the program on a single compilation unit. By default reads source text
from stdin and writes binary Kythe artifacts to stdout as a sequence of Entry
protos. Command-line arguments may be passed to the underlying compiler, if
one should exist, as positional parameters.

If -index_pack is not specified, there may be a positional parameter specified
that ends in .kindex. If one exists, no other positional parameters may be
specified, nor may an additional input parameter be specified. Input will
be read from the index file.

If -index_pack is specified, there must be exactly one positional parameter.
This parameter should be the compilation unit ID from the mounted index pack
that is meant to be indexed. No additional input parameters may be specified.

If -test_claim is specified, you may specify that one or more kindex or index
pack inputs should not produce any output by prepending the prefix "silent:"
to the input's name.

Examples:)");
  message.append(program_name +
                 " -index_pack path/to/pack/root 660f1f840000000000\n");
  message.append(program_name + " some/index.kindex\n");
  message.append(program_name + " -i foo.cc -o foo.bin -- -DINDEXING\n");
  message.append(program_name + " -i foo.cc | verifier foo.cc");
  return message;
}

bool IndexerContext::HasIndexArguments() {
  bool had_index = false;
  if (!FLAGS_index_pack.empty()) {
    had_index = true;
    CHECK(args_.size() >= 2) << "You must specify a compilation unit.";
  } else {
    for (const auto& arg : args_) {
      if (llvm::StringRef(arg).endswith(".kindex")) {
        had_index = true;
      }
    }
  }
  if (had_index) {
    CHECK_EQ("-", FLAGS_i)
        << "No other input is allowed when reading from an index file or an "
        << "index pack.";
  }
  return had_index;
}

void IndexerContext::LoadDataFromIndex(const std::string& kindex_file_or_cu) {
  jobs_.emplace_back();
  auto* job = &jobs_.back();
  std::string name = strip_silent_input_prefix(kindex_file_or_cu);
  if (name.empty()) {
    job->silent = false;
    name = kindex_file_or_cu;
  } else {
    job->silent = true;
  }
  if (!FLAGS_index_pack.empty()) {
    std::string error_text;
    auto filesystem = kythe::IndexPackPosixFilesystem::Open(
        FLAGS_index_pack, kythe::IndexPackFilesystem::OpenMode::kReadOnly,
        &error_text);
    CHECK(filesystem) << "Couldn't open index pack from " << FLAGS_index_pack
                      << ": " << error_text;
    DecodeIndexPack(name, llvm::make_unique<IndexPack>(std::move(filesystem)),
                    &job->virtual_files, &job->unit);
  } else {
    DecodeIndexFile(name, &job->virtual_files, &job->unit);
  }
  job->working_directory = job->unit.working_directory();
  if (!llvm::sys::path::is_absolute(job->working_directory)) {
    llvm::SmallString<1024> stored_wd;
    CHECK(!llvm::sys::fs::make_absolute(stored_wd));
    job->working_directory = stored_wd.str();
  }
}

void IndexerContext::LoadDataFromUnpackedFile(
    const std::string& default_filename) {
  jobs_.emplace_back();
  auto* job = &jobs_.back();
  allow_filesystem_access_ = true;
  int read_fd = STDIN_FILENO;
  std::string source_file_name = default_filename;
  llvm::SmallString<1024> cwd;
  CHECK(!llvm::sys::fs::current_path(cwd));
  job->working_directory = cwd.str();
  if (FLAGS_i != "-") {
    read_fd = open(FLAGS_i.c_str(), O_RDONLY);
    if (read_fd == -1) {
      perror("Can't open input file");
      exit(1);
    }
    source_file_name = FLAGS_i;
  }
  args_.push_back(source_file_name);
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
  job->virtual_files.push_back(std::move(file_data));
  for (const auto& arg : args_) {
    job->unit.add_argument(arg);
  }
  job->unit.mutable_v_name()->set_corpus(FLAGS_icorpus);
}

void IndexerContext::InitializeClaimClient() {
  if (!FLAGS_experimental_dynamic_claim_cache.empty()) {
    auto dynamic_claims = absl::make_unique<kythe::DynamicClaimClient>();
    dynamic_claims->set_max_redundant_claims(
        FLAGS_experimental_dynamic_overclaim);
    if (!dynamic_claims->OpenMemcache(FLAGS_experimental_dynamic_claim_cache)) {
      fprintf(stderr, "Can't open memcached\n");
      exit(1);
    }
    claim_client_ = std::move(dynamic_claims);
  } else {
    auto static_claims = absl::make_unique<kythe::StaticClaimClient>();
    if (!FLAGS_static_claim.empty()) {
      DecodeStaticClaimTable(FLAGS_static_claim, static_claims.get());
    }
    static_claims->set_process_unknown_status(FLAGS_claim_unknown);
    claim_client_ = std::move(static_claims);
  }
}

void IndexerContext::NormalizeFileVNames() {
  for (auto& job : jobs_) {
    for (auto& input : *job.unit.mutable_required_input()) {
      input.mutable_v_name()->set_path(
          CleanPath(ToStringRef(input.v_name().path())));
      input.mutable_v_name()->clear_signature();
    }
  }
}

void IndexerContext::OpenOutputStreams() {
  write_fd_ = STDOUT_FILENO;
  if (FLAGS_o != "-") {
    write_fd_ = ::open(FLAGS_o.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (write_fd_ == -1) {
      ::perror("Can't open output file");
      ::exit(1);
    }
  }
  raw_output_ =
      absl::make_unique<google::protobuf::io::FileOutputStream>(write_fd_);
  kythe_output_ = absl::make_unique<kythe::FileOutputStream>(raw_output_.get());
  kythe_output_->set_show_stats(FLAGS_cache_stats);
  kythe_output_->set_flush_after_each_entry(FLAGS_flush_after_each_entry);
}

void IndexerContext::CloseOutputStreams() {
  if (kythe_output_) {
    kythe_output_.reset();
    raw_output_.reset();
    if (::close(write_fd_) != 0) {
      ::perror("Error closing output file");
      ::exit(1);
    }
  }
}

void IndexerContext::OpenHashCache() {
  if (!FLAGS_cache.empty()) {
    auto memcache_hash_cache = llvm::make_unique<MemcachedHashCache>();
    CHECK(memcache_hash_cache->OpenMemcache(FLAGS_cache));
    memcache_hash_cache->SetSizeLimits(FLAGS_min_size, FLAGS_max_size);
    hash_cache_ = std::move(memcache_hash_cache);
  }
}

IndexerContext::IndexerContext(const std::vector<std::string>& args,
                               const std::string& default_filename)
    : args_(args), ignore_unimplemented_(FLAGS_ignore_unimplemented) {
  args_.erase(std::remove(args_.begin(), args_.end(), std::string()),
              args_.end());
  if (HasIndexArguments()) {
    for (size_t arg = 1; arg < args_.size(); ++arg) {
      LoadDataFromIndex(args[arg]);
    }
  } else {
    LoadDataFromUnpackedFile(default_filename);
  }
  if (FLAGS_normalize_file_vnames) {
    NormalizeFileVNames();
  }
  InitializeClaimClient();
  OpenOutputStreams();
  OpenHashCache();
}

IndexerContext::~IndexerContext() { CloseOutputStreams(); }

}  // namespace kythe
