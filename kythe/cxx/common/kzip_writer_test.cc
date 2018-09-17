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

#include "kythe/cxx/common/kzip_writer.h"

#include <zip.h>
#include <cstdlib>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/cxx/common/testutil.h"
#include "kythe/proto/go.pb.h"

namespace kythe {
namespace {

absl::string_view TestTmpdir() {
  return absl::StripSuffix(std::getenv("TEST_TMPDIR"), "/");
}

std::string TestFile(absl::string_view basename) {
  return absl::StrCat(TestSourceRoot(), "kythe/cxx/common/testdata/",
                      absl::StripPrefix(basename, "/"));
}

template <typename T>
struct WithStatusFn {
  bool operator()(absl::string_view digest) {
    *status = function(digest);
    return status->ok();
  }

  Status* status;
  T function;
};

template <typename T>
WithStatusFn<T> WithStatus(Status* status, T function) {
  return WithStatusFn<T>{status, std::move(function)};
}

StatusOr<std::unordered_map<std::string, std::unordered_set<std::string>>>
CopyIndex(IndexReader* reader, IndexWriter* writer) {
  Status error;
  std::unordered_map<std::string, std::unordered_set<std::string>> digests;
  Status scan = reader->Scan(WithStatus(&error, [&](absl::string_view digest) {
    auto unit = reader->ReadUnit(digest);
    if (!unit.ok()) {
      return unit.status();
    }
    auto written_digest = writer->WriteUnit(*unit);
    if (!written_digest.ok()) {
      return written_digest.status();
    }
    for (const auto& file : unit->unit().required_input()) {
      auto data = reader->ReadFile(file.info().digest());
      if (!data.ok()) {
        return data.status();
      }
      auto written_file = writer->WriteFile(*data);
      if (!written_file.ok()) {
        return written_file.status();
      }
      digests[*written_digest].insert(*written_file);
    }
    return OkStatus();
  }));
  if (!scan.ok()) {
    return scan;
  }
  if (!error.ok()) {
    return error;
  }
  return digests;
}

StatusOr<std::unordered_map<std::string, std::unordered_set<std::string>>>
ReadDigests(IndexReader* reader) {
  Status error;
  std::unordered_map<std::string, std::unordered_set<std::string>> digests;
  Status scan = reader->Scan(WithStatus(&error, [&](absl::string_view digest) {
    auto unit = reader->ReadUnit(digest);
    if (!unit.ok()) {
      return unit.status();
    }
    for (const auto& file : unit->unit().required_input()) {
      digests[std::string(digest)].insert(file.info().digest());
    }
    return OkStatus();
  }));
  if (!scan.ok()) {
    return scan;
  }
  if (!error.ok()) {
    return error;
  }
  return digests;
}

TEST(KzipReaderTest, RecapitulatesSimpleKzip) {
  // This forces the GoDetails proto descriptor to be added to the pool so we
  // can deserialize it. If we don't do this, we get an error like:
  // "Invalid type URL, unknown type: kythe.proto.GoDetails for type Any".
  proto::GoDetails needed_for_proto_deserialization;

  StatusOr<IndexReader> reader = KzipReader::Open(TestFile("stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();

  StatusOr<IndexWriter> writer =
      KzipWriter::Create(absl::StrCat(TestTmpdir(), "/stringset.kzip"));
  ASSERT_TRUE(writer.ok()) << writer.status();
  auto written_digests = CopyIndex(&*reader, &*writer);
  ASSERT_TRUE(written_digests.ok()) << written_digests.status();
  {
    auto status = writer->Close();
    ASSERT_TRUE(status.ok()) << status;
  }

  reader = KzipReader::Open(absl::StrCat(TestTmpdir(), "/stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();
  auto read_digests = ReadDigests(&*reader);
  ASSERT_TRUE(read_digests.ok());
  EXPECT_EQ(*written_digests, *read_digests);
}

}  // namespace
}  // namespace kythe
