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

// kindex_tool: convert between .kindex files and ASCII protocol buffers.
//
// kindex_tool -explode some/file.kindex
//   dumps some/file.kindex to some/file.kindex_UNIT, some/file.kindex_sha2...
//   as ascii protobufs
// kindex_tool -assemble some/file.kindex some/unit some/content...
//   assembles some/file.kindex using some/unit as the CompilationUnit and
//   any other input files as FileData

#include <sys/stat.h>
#include <fcntl.h>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/text_format.h"
#include "kythe/proto/analysis.pb.h"

DEFINE_string(assemble, "", "Assemble positional args into output file");
DEFINE_string(explode, "", "Explode this kindex file into its constituents");
DEFINE_bool(canonicalize_hashes, false,
            "Replace transcripts with sequence numbers");
DEFINE_bool(suppress_details, false, "Suppress CU details.");

/// \brief Gives each `hash` a unique, shorter ID based on visitation order.
static void CanonicalizeHash(std::map<google::protobuf::string, size_t>* hashes,
                             google::protobuf::string* hash) {
  auto inserted = hashes->insert(std::make_pair(*hash, hashes->size()));
  *hash =
      google::protobuf::string("hash" + std::to_string(inserted.first->second));
}

static void DumpIndexFile(const std::string& path) {
  using namespace google::protobuf::io;
  int in_fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(in_fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(in_fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  CodedInputStream coded_input_stream(&gzip_input_stream);
  google::protobuf::uint32 byte_size;
  bool decoded_unit = false;
  std::map<google::protobuf::string, size_t> hash_table;
  while (coded_input_stream.ReadVarint32(&byte_size)) {
    auto limit = coded_input_stream.PushLimit(byte_size);
    if (!decoded_unit) {
      kythe::proto::CompilationUnit unit;
      CHECK(unit.ParseFromCodedStream(&coded_input_stream));
      std::string out_path = path + "_UNIT";
      int out_fd = open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                        S_IREAD | S_IWRITE);
      CHECK_GE(out_fd, 0) << "Couldn't open " << out_path << " for writing.";
      if (FLAGS_suppress_details) {
        unit.clear_details();
      }
      if (FLAGS_canonicalize_hashes) {
        CanonicalizeHash(&hash_table, unit.mutable_entry_context());
        for (int i = 0; i < unit.required_input_size(); ++i) {
          auto* input = unit.mutable_required_input(i);
          for (int r = 0; r < input->context_size(); ++r) {
            auto* row = input->mutable_context(r);
            CanonicalizeHash(&hash_table, row->mutable_source_context());
            for (int c = 0; c < row->column_size(); ++c) {
              auto* col = row->mutable_column(c);
              CanonicalizeHash(&hash_table, col->mutable_linked_context());
            }
          }
        }
      }
      FileOutputStream file_output_stream(out_fd);
      CHECK(google::protobuf::TextFormat::Print(unit, &file_output_stream));
      CHECK(file_output_stream.Close());
      decoded_unit = true;
    } else {
      kythe::proto::FileData content;
      CHECK(content.ParseFromCodedStream(&coded_input_stream));
      CHECK(content.has_info() && !content.info().digest().empty());
      std::string out_path = path + "_" + content.info().digest();
      int out_fd = open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                        S_IREAD | S_IWRITE);
      CHECK_GE(out_fd, 0) << "Couldn't open " << out_path << " for writing.";
      FileOutputStream file_output_stream(out_fd);
      CHECK(google::protobuf::TextFormat::Print(content, &file_output_stream));
      CHECK(file_output_stream.Close());
    }
    coded_input_stream.PopLimit(limit);
  }
  CHECK(file_input_stream.Close());
}

static void BuildIndexFile(const std::string& outfile,
                           const std::vector<std::string>& elements) {
  CHECK(!elements.empty()) << "Need at least a CompilationUnit!";
  using namespace google::protobuf::io;
  int out_fd =
      open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  CHECK(out_fd >= 0) << "Couldn't open " << outfile << " for writing.";
  {
    FileOutputStream file_output_stream(out_fd);
    GzipOutputStream::Options options;
    options.format = GzipOutputStream::GZIP;
    GzipOutputStream gzip_stream(&file_output_stream, options);
    CodedOutputStream coded_stream(&gzip_stream);

    kythe::proto::CompilationUnit unit;
    int in_fd = open(elements[0].c_str(), O_RDONLY, S_IREAD | S_IWRITE);
    CHECK_GE(in_fd, 0) << "Couldn't open input file " << elements[0];
    FileInputStream file_input_stream(in_fd);
    CHECK(google::protobuf::TextFormat::Parse(&file_input_stream, &unit));
    coded_stream.WriteVarint32(unit.ByteSize());
    CHECK(unit.SerializeToCodedStream(&coded_stream));
    CHECK(file_input_stream.Close());

    for (size_t i = 1; i < elements.size(); ++i) {
      kythe::proto::FileData content;
      int in_fd = open(elements[i].c_str(), O_RDONLY, S_IREAD | S_IWRITE);
      CHECK_GE(in_fd, 0) << "Couldn't open input file " << elements[i];
      FileInputStream file_input_stream(in_fd);
      CHECK(google::protobuf::TextFormat::Parse(&file_input_stream, &content));
      coded_stream.WriteVarint32(content.ByteSize());
      CHECK(content.SerializeToCodedStream(&coded_stream));
      CHECK(file_input_stream.Close());
    }
    CHECK(!coded_stream.HadError());
  }
  CHECK(close(out_fd) == 0);
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetVersionString("0.1");
  google::SetUsageMessage(R"(kindex_tool: work with .kindex files
kindex_tool -explode some/file.kindex
  dumps some/file.kindex to some/file.kindex_UNIT, some/file.kindex_sha2...
  as ascii protobufs

kindex_tool -assemble some/file.kindex some/unit some/content...
  assembles some/file.kindex using some/unit as the CompilationUnit and
  any other input files as FileData)");
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (!FLAGS_explode.empty()) {
    DumpIndexFile(FLAGS_explode);
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
