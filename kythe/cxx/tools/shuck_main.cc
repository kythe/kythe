/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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
// shuck: a tool to slice an index pack using a claim database
//
// bazel run //kythe/cxx/tools:shuck -- \
//     -index_pack ~/linux/linux-3.19-rc6/kernel-pack
//     -static_claim ~/linux/linux-3.19-rc6/kernel.claims
//     include/linux/hid-debug.h
// will print all the units that depend on include/linux/hid-debug.h and call
// out the ones that claim one of its transcripts with a #:
//
// units/dd2807208a1350357673bb218def55f50ca3a0df37682d308d51ad5af961ecbf.unit
// # prev claim contains drivers/hid/hid-core.c
// units/b73d40cc9e012fed3c51ab55592906c2821efea08ab63dfb286c80c42394ffa3.unit
// units/f6813e10b2dd3b2052b58ecc86b7035afd48dbd439cf998f7ff783f395c81974.unit
// units/aafe375e78e80ad6d27f75e26a5987068b57d0baded5a549964ee09191667f2b.unit
//
// The -slice_dependencies option will list all of the compilation units
// making claims on the paths you pass in as well as all of the data files
// on which they depend.

#include <fcntl.h>
#include <sys/stat.h>

#include <set>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/extractor/index_pack.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/claim.pb.h"

using kythe::proto::ClaimAssignment;
using kythe::proto::CompilationUnit;

ABSL_FLAG(std::string, index_pack, "", "Read from this index pack.");
ABSL_FLAG(std::string, static_claim, "", "Read from this claim file.");
ABSL_FLAG(bool, slice_dependencies, false, "Describe a miminal index pack.");

template <typename ProtoType, typename ClosureType>
void ReadGzippedDelimitedProtoSequence(const std::string& path, ClosureType f) {
  namespace io = ::google::protobuf::io;
  int fd = ::open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path << ": "
                  << ::strerror(errno);
  io::FileInputStream file_input_stream(fd);
  io::GzipInputStream gzip_input_stream(&file_input_stream);
  google::protobuf::uint32 byte_size;
  for (;;) {
    io::CodedInputStream coded_input_stream(&gzip_input_stream);
    coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
    if (!coded_input_stream.ReadVarint32(&byte_size)) {
      break;
    }
    coded_input_stream.PushLimit(byte_size);
    ProtoType proto;
    CHECK(proto.ParseFromCodedStream(&coded_input_stream));
    f(proto);
  }
  ::close(fd);
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  CHECK(!absl::GetFlag(FLAGS_index_pack).empty()) << "Need an index pack.";
  CHECK(!absl::GetFlag(FLAGS_static_claim).empty())
      << "Need a static claim table.";
  const std::set<std::string> paths(remain.begin() + 1, remain.end());
  CHECK(!paths.empty()) << "Specify one or more paths.";
  std::set<std::string> compilations;
  ReadGzippedDelimitedProtoSequence<ClaimAssignment>(
      absl::GetFlag(FLAGS_static_claim),
      [&compilations, &paths](const ClaimAssignment& claim) {
        if (paths.count(claim.dependency_v_name().path())) {
          compilations.insert(claim.compilation_v_name().signature());
        }
      });
  std::string error_text;
  auto filesystem = kythe::IndexPackPosixFilesystem::Open(
      absl::GetFlag(FLAGS_index_pack),
      kythe::IndexPackFilesystem::OpenMode::kReadOnly, &error_text);
  if (!filesystem) {
    absl::FPrintF(stderr, "Error reading index pack: %s\n", error_text);
    return 1;
  }
  kythe::IndexPack pack(std::move(filesystem));
  if (!pack.ScanData(
          kythe::IndexPackFilesystem::DataKind::kCompilationUnit,
          [&pack, &paths, &compilations](const std::string& file_id) {
            std::string error_text;
            CompilationUnit unit;
            CHECK(pack.ReadCompilationUnit(file_id, &unit, &error_text))
                << "Error reading unit " << file_id << ": " << error_text;
            bool claimed = compilations.count(unit.v_name().signature());
            for (const auto& input : unit.required_input()) {
              if (paths.count(input.v_name().path())) {
                if (!absl::GetFlag(FLAGS_slice_dependencies) || claimed) {
                  absl::PrintF("units/%s.unit\n", file_id);
                  if (!absl::GetFlag(FLAGS_slice_dependencies) && claimed) {
                    absl::PrintF("# prev claim contains");
                    for (const auto& arg : unit.source_file()) {
                      absl::PrintF(" %s", arg);
                    }
                    absl::PrintF("\n");
                  }
                }
              }
              if (absl::GetFlag(FLAGS_slice_dependencies) && claimed) {
                absl::PrintF("files/%s.data\n", input.info().digest());
              }
            }
            return true;
          },
          &error_text)) {
    absl::FPrintF(stderr, "Error scanning index pack: %s\n", error_text);
  }
  return 0;
}
