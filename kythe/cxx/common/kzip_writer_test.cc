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
#include <unordered_set>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/cxx/common/testutil.h"
#include "kythe/proto/go.pb.h"

namespace kythe {
namespace {
using ::testing::ElementsAre;

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

std::string TestOutputFile(absl::string_view basename) {
  const auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
  return absl::StrCat(TestTmpdir(), "/", test_info->test_case_name(), "_",
                      test_info->name(), "_", basename);
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

TEST(KzipWriterTest, RecapitulatesSimpleKzip) {
  // This forces the GoDetails proto descriptor to be added to the pool so we
  // can deserialize it. If we don't do this, we get an error like:
  // "Invalid type URL, unknown type: kythe.proto.GoDetails for type Any".
  proto::GoDetails needed_for_proto_deserialization;

  StatusOr<IndexReader> reader = KzipReader::Open(TestFile("stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();

  std::string output_file = TestOutputFile("stringset.kzip");
  StatusOr<IndexWriter> writer = KzipWriter::Create(output_file);
  ASSERT_TRUE(writer.ok()) << writer.status();
  auto written_digests = CopyIndex(&*reader, &*writer);
  ASSERT_TRUE(written_digests.ok()) << written_digests.status();
  {
    auto status = writer->Close();
    ASSERT_TRUE(status.ok()) << status;
  }

  reader = KzipReader::Open(output_file);
  ASSERT_TRUE(reader.ok()) << reader.status();
  auto read_digests = ReadDigests(&*reader);
  ASSERT_TRUE(read_digests.ok()) << read_digests.status();
  EXPECT_EQ(*written_digests, *read_digests);
}

TEST(KzipWriterTest, IncludesDirectoryEntries) {
  std::string dummy_file = TestOutputFile("dummy.kzip");
  StatusOr<IndexWriter> writer = KzipWriter::Create(dummy_file);
  ASSERT_TRUE(writer.ok()) << writer.status();
  {
    auto digest = writer->WriteFile("contents");
    ASSERT_TRUE(digest.ok()) << digest.status();
  }
  {
    auto status = writer->Close();
    ASSERT_TRUE(status.ok()) << status;
  }

  std::vector<std::string> contents;
  {
    auto* archive = zip_open(dummy_file.c_str(), ZIP_RDONLY, nullptr);
    ASSERT_NE(archive, nullptr);
    struct Closer {
      ~Closer() { zip_discard(a); }
      zip_t* a;
    } closer{archive};
    for (int i = 0; i < zip_get_num_entries(archive, 0); ++i) {
      const char* name = zip_get_name(archive, i, 0);
      ASSERT_NE(archive, nullptr) << libzip::ToStatus(zip_get_error(archive));
      contents.push_back(name);
    }
  }
  EXPECT_THAT(
      contents,
      // Order matters here as "root/" must come first.
      // We don't really care about the rest of the entries, but it's easy
      // enough to fix the order of the subdirectories and minimally harmful.
      ElementsAre(
          "root/", "root/units/", "root/files/",
          "root/files/"
          "d1b2a59fbea7e20077af9f91b27e95e865061b270be03ff539ab3b73587882e8"));
}

TEST(KzipWriterTest, DuplicateFilesAreIgnored) {
  StatusOr<IndexWriter> writer =
      KzipWriter::Create(TestOutputFile("dummy.kzip"));
  ASSERT_TRUE(writer.ok()) << writer.status();
  {
    auto digest = writer->WriteFile("contents");
    ASSERT_TRUE(digest.ok()) << digest.status();
  }
  {
    auto digest = writer->WriteFile("contents");
    ASSERT_TRUE(digest.ok()) << digest.status();
  }
  {
    auto status = writer->Close();
    ASSERT_TRUE(status.ok()) << status;
  }
}

}  // namespace
}  // namespace kythe
