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

#include "kythe/cxx/indexer/cxx/frontend.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdint>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/memory/memory.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/indexing/MemcachedHashCache.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/indexer/cxx/DynamicClaimClient.h"
#include "kythe/proto/buildinfo.pb.h"
#include "kythe/proto/claim.pb.h"
#include "llvm/ADT/STLExtras.h"

ABSL_FLAG(std::string, o, "-", "Output filename");
ABSL_FLAG(std::string, i, "-", "Input filename");
ABSL_FLAG(bool, ignore_unimplemented, true,
          "Continue indexing even if we find something we don't support.");
ABSL_FLAG(bool, flush_after_each_entry, true,
          "Flush output after writing each entry.");
ABSL_FLAG(std::string, static_claim, "", "Use a static claim table.");
ABSL_FLAG(bool, claim_unknown, true,
          "Process files with unknown claim status.");
ABSL_FLAG(std::string, cache, "",
          "Use a memcache instance (ex: \"--SERVER=foo:1234\")");
ABSL_FLAG(int32_t, min_size, 4096, "Minimum size of an entry bundle");
ABSL_FLAG(int32_t, max_size, 1024 * 32, "Maximum size of an entry bundle");
ABSL_FLAG(bool, cache_stats, false, "Show cache stats");
ABSL_FLAG(std::string, icorpus, "",
          "Corpus to use for files specified with -i");
ABSL_FLAG(std::string, ibuild_config, "",
          "Build config to use for files specified with -i");
ABSL_FLAG(bool, normalize_file_vnames, false,
          "Normalize incoming file vnames.");
ABSL_FLAG(std::string, experimental_dynamic_claim_cache, "",
          "Use a memcache instance for dynamic claims (EXPERIMENTAL)");
// Setting this to a value > 1 allows the same object (e.g., a transcript of
// an include file) to be claimed multiple times. In the absence of transcript
// labels, setting this value to 1 means that only one environment will be
// considered when indexing a vname. This may result in (among other effects)
// conditionally included code never being indexed if the symbols checked differ
// between translation units.
ABSL_FLAG(uint64_t, experimental_dynamic_overclaim, 1,
          "Maximum number of dynamic claims per claimable (EXPERIMENTAL)");
ABSL_FLAG(bool, test_claim, false,
          "Use an in-memory claim database for testing.");
namespace kythe {

namespace {
/// The prefix prepended to silent inputs. Only checked when "--test_claim"
/// is enabled.
constexpr char kSilentPrefix[] = "silent:";

/// The message type URI for the build details message.
constexpr char kBuildDetailsURI[] = "kythe.io/proto/kythe.proto.BuildDetails";

/// \return the input name stripped of its prefix if it's silent; an empty
/// string otherwise.
llvm::StringRef strip_silent_input_prefix(llvm::StringRef argument) {
  if (absl::GetFlag(FLAGS_test_claim) && argument.startswith(kSilentPrefix)) {
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
    coded_input_stream.SetTotalBytesLimit(INT_MAX);
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

/// \brief Normalize input file vnames by cleaning paths and clearing
/// signatures.
void MaybeNormalizeFileVNames(IndexerJob* job) {
  if (!absl::GetFlag(FLAGS_normalize_file_vnames)) {
    return;
  }
  for (auto& input : *job->unit.mutable_required_input()) {
    input.mutable_v_name()->set_path(CleanPath(input.v_name().path()));
    input.mutable_v_name()->clear_signature();
  }
}

/// \brief Reads all compilations from a .kzip file into memory.
/// \param path The path from which the file should be read.
/// \param jobs A vector to add a job to for each compilation in the kzip.
/// \param silent The silent flag is copied to each of the jobs created from the
/// kzip file.
void DecodeKZipFile(const std::string& path, bool silent,
                    const IndexerContext::CompilationVisitCallback& visit) {
  absl::StatusOr<IndexReader> reader = kythe::KzipReader::Open(path);
  CHECK(reader.ok()) << "Couldn't open kzip from " << path << ": "
                     << reader.status();
  bool compilation_read = false;
  auto status = reader->Scan([&](absl::string_view digest) {
    IndexerJob job;
    job.silent = silent;

    auto compilation = reader->ReadUnit(digest);
    for (const auto& file : compilation->unit().required_input()) {
      auto content = reader->ReadFile(file.info().digest());
      CHECK(content.ok()) << "Unable to read file with digest: "
                          << file.info().digest() << ": " << content.status();
      proto::FileData file_data;
      file_data.set_content(*content);
      file_data.mutable_info()->set_path(file.info().path());
      file_data.mutable_info()->set_digest(file.info().digest());
      job.virtual_files.push_back(std::move(file_data));
    }
    job.unit = compilation->unit();

    MaybeNormalizeFileVNames(&job);
    visit(job);

    compilation_read = true;
    return true;
  });
  CHECK(status.ok()) << status.ToString();
  CHECK(compilation_read) << "Missing compilation in " << path;
}
}  // anonymous namespace

std::string IndexerContext::UsageMessage(const std::string& program_title,
                                         const std::string& program_name) {
  std::string message = "Command-line frontend for " + program_title;
  message.append(R"(.
Invokes the program on compilation unit(s). By default reads source text from
stdin and writes binary Kythe artifacts to stdout as a sequence of Entry
protos. Command-line arguments may be passed to the underlying compiler, if
one should exist, as positional parameters.

There may be a positional parameter specified that ends in .kzip. If one exists,
no other positional parameters may be specified, nor may an additional input
parameter be specified. Input will be read from the kzip file.

If -test_claim is specified, you may specify that one or more kzip inputs
should not produce any output by prepending the prefix "silent:" to the input's
name.

Examples:)");
  message.append(program_name + " some/index.kzip\n");
  message.append(program_name + " -i foo.cc -o foo.bin -- -DINDEXING\n");
  message.append(program_name + " -i foo.cc | verifier foo.cc");
  return message;
}

bool IndexerContext::HasIndexArguments() {
  for (const auto& arg : args_) {
    auto path = llvm::StringRef(arg);
    if (path.endswith(".kzip")) {
      CHECK_EQ("-", absl::GetFlag(FLAGS_i))
          << "No other input is allowed when reading from a kzip file.";
      return true;
    }
  }
  return false;
}

void IndexerContext::LoadDataFromKZip(const std::string& file_or_cu,
                                      const CompilationVisitCallback& visit) {
  std::string name(strip_silent_input_prefix(file_or_cu));
  const bool silent = !name.empty();
  if (name.empty()) {
    name = file_or_cu;
  }
  CHECK(llvm::StringRef(file_or_cu).endswith(".kzip"));
  DecodeKZipFile(name, silent, visit);
}

void IndexerContext::LoadDataFromUnpackedFile(
    const std::string& default_filename,
    const CompilationVisitCallback& visit) {
  IndexerJob job;
  int read_fd = STDIN_FILENO;
  std::string source_file_name = default_filename;
  llvm::SmallString<1024> cwd;
  CHECK(!llvm::sys::fs::current_path(cwd));
  job.unit.set_working_directory(std::string(cwd.str()));
  if (absl::GetFlag(FLAGS_i) != "-") {
    source_file_name = absl::GetFlag(FLAGS_i);
    read_fd = open(source_file_name.c_str(), O_RDONLY);
    if (read_fd == -1) {
      perror("Can't open input file");
      exit(1);
    }
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
  file_data.set_content(std::string(source_data.str()));
  job.virtual_files.push_back(std::move(file_data));
  job.unit.add_source_file(source_file_name);
  for (const auto& arg : args_) {
    job.unit.add_argument(arg);
  }
  job.unit.mutable_v_name()->set_corpus(absl::GetFlag(FLAGS_icorpus));
  if (!absl::GetFlag(FLAGS_ibuild_config).empty()) {
    proto::BuildDetails details;
    details.set_build_config(absl::GetFlag(FLAGS_ibuild_config));
    auto* any = job.unit.add_details();
    any->PackFrom(details);
    any->set_type_url(kBuildDetailsURI);
  }
  MaybeNormalizeFileVNames(&job);
  visit(job);
}

void IndexerContext::InitializeClaimClient() {
  if (!absl::GetFlag(FLAGS_experimental_dynamic_claim_cache).empty()) {
    auto dynamic_claims = std::make_unique<kythe::DynamicClaimClient>();
    dynamic_claims->set_max_redundant_claims(
        absl::GetFlag(FLAGS_experimental_dynamic_overclaim));
    CHECK(dynamic_claims->OpenMemcache(
        absl::GetFlag(FLAGS_experimental_dynamic_claim_cache)))
        << "Can't open memcached";
    claim_client_ = std::move(dynamic_claims);
  } else {
    auto static_claims = std::make_unique<kythe::StaticClaimClient>();
    if (!absl::GetFlag(FLAGS_static_claim).empty()) {
      DecodeStaticClaimTable(absl::GetFlag(FLAGS_static_claim),
                             static_claims.get());
    }
    static_claims->set_process_unknown_status(
        absl::GetFlag(FLAGS_claim_unknown));
    claim_client_ = std::move(static_claims);
  }
}

void IndexerContext::OpenOutputStreams() {
  write_fd_ = STDOUT_FILENO;
  if (absl::GetFlag(FLAGS_o) != "-") {
    write_fd_ =
        ::open(absl::GetFlag(FLAGS_o).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (write_fd_ == -1) {
      ::perror("Can't open output file");
      ::exit(1);
    }
  }
  raw_output_ =
      std::make_unique<google::protobuf::io::FileOutputStream>(write_fd_);
  kythe_output_ = std::make_unique<kythe::FileOutputStream>(raw_output_.get());
  kythe_output_->set_show_stats(absl::GetFlag(FLAGS_cache_stats));
  kythe_output_->set_flush_after_each_entry(
      absl::GetFlag(FLAGS_flush_after_each_entry));
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
  if (!absl::GetFlag(FLAGS_cache).empty()) {
    auto memcache_hash_cache = std::make_unique<MemcachedHashCache>();
    CHECK(memcache_hash_cache->OpenMemcache(absl::GetFlag(FLAGS_cache)));
    memcache_hash_cache->SetSizeLimits(absl::GetFlag(FLAGS_min_size),
                                       absl::GetFlag(FLAGS_max_size));
    hash_cache_ = std::move(memcache_hash_cache);
  }
}

IndexerContext::IndexerContext(const std::vector<std::string>& args,
                               const std::string& default_filename)
    : args_(args),
      default_filename_(default_filename),
      ignore_unimplemented_(absl::GetFlag(FLAGS_ignore_unimplemented)) {
  args_.erase(std::remove(args_.begin(), args_.end(), std::string()),
              args_.end());
  unpacked_inputs_ = !HasIndexArguments();

  InitializeClaimClient();
  OpenOutputStreams();
  OpenHashCache();
}

IndexerContext::~IndexerContext() { CloseOutputStreams(); }

void IndexerContext::EnumerateCompilations(
    const CompilationVisitCallback& visit) {
  if (unpacked_inputs_) {
    LoadDataFromUnpackedFile(default_filename_, visit);
  } else {
    for (size_t arg = 1; arg < args_.size(); ++arg) {
      LoadDataFromKZip(args_[arg], visit);
    }
  }
}

}  // namespace kythe
