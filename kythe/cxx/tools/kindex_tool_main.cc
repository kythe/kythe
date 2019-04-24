/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// kindex_tool: convert between .kindex files and ASCII protocol buffers.
//
// kindex_tool -explode some/file.kindex
//   dumps some/file.kindex to some/file.kindex_UNIT, some/file.kindex_sha2...
//   as ascii protobufs
// kindex_tool -assemble some/file.kindex some/unit some/content...
//   assembles some/file.kindex using some/unit as the CompilationUnit and
//   any other input files as FileData

#include <fcntl.h>
#include <sys/stat.h>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/match.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/text_format.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/buildinfo.pb.h"
#include "kythe/proto/cxx.pb.h"
#include "kythe/proto/filecontext.pb.h"
#include "re2/re2.h"

DEFINE_string(assemble, "", "Assemble positional args into output file");
DEFINE_string(explode, "", "Explode this kindex file into its constituents");
DEFINE_bool(canonicalize_hashes, false,
            "Replace transcripts with sequence numbers");
DEFINE_bool(suppress_details, false, "Suppress CU details.");
DEFINE_string(keep_details_matching, "",
              "If present, include these details when suppressing the rest.");

namespace {

/// \brief Range wrapper around ContextDependentVersion, if any.
class MutableContextRows {
 public:
  using iterator =
      decltype(std::declval<kythe::proto::ContextDependentVersion>()
                   .mutable_row()
                   ->begin());
  explicit MutableContextRows(
      kythe::proto::CompilationUnit::FileInput* file_input) {
    for (google::protobuf::Any& detail : *file_input->mutable_details()) {
      if (detail.UnpackTo(&context_)) {
        any_ = &detail;
      }
    }
  }

  ~MutableContextRows() {
    if (any_ != nullptr) {
      any_->PackFrom(context_);
    }
  }

  iterator begin() { return context_.mutable_row()->begin(); }
  iterator end() { return context_.mutable_row()->end(); }

 private:
  google::protobuf::Any* any_ = nullptr;
  kythe::proto::ContextDependentVersion context_;
};

class PermissiveFinder : public google::protobuf::TextFormat::Finder {
 public:
  const google::protobuf::Descriptor* FindAnyType(
      const google::protobuf::Message& message, const std::string& prefix,
      const std::string& name) const {
    // Ignore any provided prefix and use one of the default supported ones.
    return Finder::FindAnyType(message, "type.googleapis.com/", name);
  }
};

/// \brief Gives each `hash` a unique, shorter ID based on visitation order.
void CanonicalizeHash(
    absl::flat_hash_map<google::protobuf::string, size_t>* hashes,
    google::protobuf::string* hash) {
  auto inserted = hashes->insert({*hash, hashes->size()});
  *hash =
      google::protobuf::string("hash" + std::to_string(inserted.first->second));
}

void DumpCompilationUnit(const std::string& path,
                         kythe::proto::CompilationUnit* unit) {
  absl::flat_hash_map<google::protobuf::string, size_t> hash_table;
  std::string out_path = path + "_UNIT";
  int out_fd =
      open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  CHECK_GE(out_fd, 0) << "Couldn't open " << out_path << " for writing.";
  if (FLAGS_suppress_details) {
    if (FLAGS_keep_details_matching.empty()) {
      unit->clear_details();
    } else {
      google::protobuf::RepeatedPtrField<google::protobuf::Any> keep;
      re2::RE2 detail_pattern(FLAGS_keep_details_matching);
      for (const auto& detail : *unit->mutable_details()) {
        if (re2::RE2::FullMatch(detail.type_url(), detail_pattern)) {
          *keep.Add() = detail;
        }
      }
      unit->mutable_details()->Swap(&keep);
    }
  }
  if (FLAGS_canonicalize_hashes) {
    CanonicalizeHash(&hash_table, unit->mutable_entry_context());
    for (auto& input : *unit->mutable_required_input()) {
      for (auto& row : MutableContextRows(&input)) {
        CanonicalizeHash(&hash_table, row.mutable_source_context());
        for (auto& column : *row.mutable_column()) {
          CanonicalizeHash(&hash_table, column.mutable_linked_context());
        }
      }
    }
  }
  google::protobuf::io::FileOutputStream file_output_stream(out_fd);
  google::protobuf::TextFormat::Printer printer;
  printer.SetExpandAny(true);
  PermissiveFinder finder;
  printer.SetFinder(&finder);
  CHECK(printer.Print(*unit, &file_output_stream));
  CHECK(file_output_stream.Close());
}

void DumpFileData(const std::string& path,
                  const kythe::proto::FileData& content) {
  CHECK(content.has_info() && !content.info().digest().empty());
  std::string out_path = path + "_" + content.info().digest();
  int out_fd =
      open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  CHECK_GE(out_fd, 0) << "Couldn't open " << out_path << " for writing.";
  google::protobuf::io::FileOutputStream file_output_stream(out_fd);
  google::protobuf::TextFormat::Printer printer;
  printer.SetExpandAny(true);
  PermissiveFinder finder;
  printer.SetFinder(&finder);
  CHECK(printer.Print(content, &file_output_stream));
  CHECK(file_output_stream.Close());
}

void DumpIndexFile(const std::string& path) {
  int in_fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(in_fd, 0) << "Couldn't open input file " << path;
  google::protobuf::io::FileInputStream file_input_stream(in_fd);
  google::protobuf::io::GzipInputStream gzip_input_stream(&file_input_stream);
  google::protobuf::uint32 byte_size;
  bool decoded_unit = false;
  for (;;) {
    google::protobuf::io::CodedInputStream coded_input_stream(
        &gzip_input_stream);
    coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
    if (!coded_input_stream.ReadVarint32(&byte_size)) {
      break;
    }
    coded_input_stream.PushLimit(byte_size);
    if (!decoded_unit) {
      kythe::proto::CompilationUnit unit;
      CHECK(unit.ParseFromCodedStream(&coded_input_stream));
      DumpCompilationUnit(path, &unit);
      decoded_unit = true;
    } else {
      kythe::proto::FileData content;
      CHECK(content.ParseFromCodedStream(&coded_input_stream));
      DumpFileData(path, content);
    }
  }
  CHECK(file_input_stream.Close());
}

void DumpKzipFile(const std::string& path) {
  kythe::StatusOr<kythe::IndexReader> reader = kythe::KzipReader::Open(path);
  CHECK(reader) << "Couldn't open kzip from " << path;
  auto status = reader->Scan([&](absl::string_view digest) {
    auto compilation = reader->ReadUnit(digest);
    CHECK(compilation) << "Couldn't get compilation for " << digest << ": "
                       << compilation.status();
    DumpCompilationUnit(path, compilation->mutable_unit());
    for (const auto& file : compilation->unit().required_input()) {
      auto content = reader->ReadFile(file.info().digest());
      CHECK(content) << "Unable to read file with digest: "
                     << file.info().digest() << ": " << content.status();
      kythe::proto::FileData file_data;
      file_data.set_content(*content);
      file_data.mutable_info()->set_path(file.info().path());
      file_data.mutable_info()->set_digest(file.info().digest());
      DumpFileData(path, file_data);
    }
    return true;
  });
  CHECK(status.ok()) << status.ToString();
}

void BuildIndexFile(const std::string& outfile,
                    const std::vector<std::string>& elements) {
  CHECK(!elements.empty()) << "Need at least a CompilationUnit!";
  int out_fd =
      open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  CHECK(out_fd >= 0) << "Couldn't open " << outfile << " for writing.";
  {
    google::protobuf::io::FileOutputStream file_output_stream(out_fd);
    google::protobuf::io::GzipOutputStream::Options options;
    options.format = google::protobuf::io::GzipOutputStream::GZIP;
    google::protobuf::io::GzipOutputStream gzip_stream(&file_output_stream,
                                                       options);
    google::protobuf::io::CodedOutputStream coded_stream(&gzip_stream);

    kythe::proto::CompilationUnit unit;
    int in_fd = open(elements[0].c_str(), O_RDONLY, S_IREAD | S_IWRITE);
    CHECK_GE(in_fd, 0) << "Couldn't open input file " << elements[0];
    google::protobuf::io::FileInputStream file_input_stream(in_fd);
    CHECK(google::protobuf::TextFormat::Parse(&file_input_stream, &unit));
    coded_stream.WriteVarint32(unit.ByteSize());
    CHECK(unit.SerializeToCodedStream(&coded_stream));
    CHECK(file_input_stream.Close());

    for (size_t i = 1; i < elements.size(); ++i) {
      kythe::proto::FileData content;
      int in_fd = open(elements[i].c_str(), O_RDONLY, S_IREAD | S_IWRITE);
      CHECK_GE(in_fd, 0) << "Couldn't open input file " << elements[i];
      google::protobuf::io::FileInputStream file_input_stream(in_fd);
      CHECK(google::protobuf::TextFormat::Parse(&file_input_stream, &content));
      coded_stream.WriteVarint32(content.ByteSize());
      CHECK(content.SerializeToCodedStream(&coded_stream));
      CHECK(file_input_stream.Close());
    }
    CHECK(!coded_stream.HadError());
  }
  CHECK(close(out_fd) == 0);
}

}  // namespace

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::proto::CxxCompilationUnitDetails link_cxx_details;
  kythe::proto::BuildDetails link_build_details;
  google::InitGoogleLogging(argv[0]);
  gflags::SetVersionString("0.1");
  gflags::SetUsageMessage(R"(kindex_tool: work with .kindex files
kindex_tool -explode some/file.kindex (or .kzip)
  dumps some/file.kindex to some/file.kindex_UNIT, some/file.kindex_sha2...
  as ascii protobufs

kindex_tool -assemble some/file.kindex some/unit some/content...
  assembles some/file.kindex using some/unit as the CompilationUnit and
  any other input files as FileData)");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (!FLAGS_explode.empty()) {
    if (absl::EndsWith(FLAGS_explode, ".kzip")) {
      DumpKzipFile(FLAGS_explode);
    } else {
      DumpIndexFile(FLAGS_explode);
    }
  } else if (!FLAGS_assemble.empty()) {
    CHECK(argc >= 2) << "Need at least the unit.";
    std::vector<std::string> constituent_parts(argv + 1, argv + argc);
    BuildIndexFile(FLAGS_assemble, constituent_parts);
  } else {
    fprintf(stderr, "Specify either -assemble or -explode.\n");
    return -1;
  }
  return 0;
}
