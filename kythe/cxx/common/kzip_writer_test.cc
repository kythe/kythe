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

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/libzip/error.h"

namespace kythe {
namespace {

absl::string_view TestSrcdir() {
  return absl::StripSuffix(CHECK_NOTNULL(getenv("TEST_SRCDIR")), "/");
}

absl::string_view TestTmpdir() {
  return absl::StripSuffix(CHECK_NOTNULL(getenv("TEST_TMPDIR")), "/");
}

std::string TestFile(absl::string_view basename) {
  return absl::StrCat(TestSrcdir(), "/io_kythe/kythe/cxx/common/testdata/",
                      absl::StripPrefix(basename, "/"));
}

template <typename T> struct WithStatusFn {
  bool operator()(absl::string_view digest) {
    *status = function(digest);
    return status->ok();
  }

  Status* status;
  T function;
};

template <typename T> WithStatusFn<T> WithStatus(Status* status, T function) {
  return WithStatusFn<T>{status, std::move(function)};
}

TEST(KzipReaderTest, RecapitulatesSimpleKzip) {
  StatusOr<IndexReader> reader = KzipReader::Open(TestFile("stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();

  StatusOr<IndexWriter> writer =
      KzipWriter::Create(absl::StrCat(TestTmpdir(), "/stringset.kzip"));
  ASSERT_TRUE(writer.ok()) << writer.status();
  Status error;
  std::unordered_map<std::string, std::unordered_set<std::string>> digests;
  EXPECT_TRUE(
      reader
          ->Scan(WithStatus(
              &error,
              [&](absl::string_view digest) {
                if (auto unit = reader->ReadUnit(digest)) {
                  if (auto written = writer->WriteUnit(*unit)) {
                    for (const auto& file : unit->unit().required_input()) {
                      if (auto data = reader->ReadFile(file.info().digest())) {
                        if (auto written_file = writer->WriteFile(*data)) {
                          EXPECT_EQ(*written_file, file.info().digest());
                          digests[*written].insert(*written_file);
                        } else {
                          return written_file.status();
                        }
                      } else {
                        return data.status();
                      }
                    }
                  } else {
                    return written.status();
                  }
                } else {
                  return unit.status();
                }
                return OkStatus();
              }))
          .ok());
  EXPECT_TRUE(error.ok()) << error;
  {
    auto status = writer->Close();
    ASSERT_TRUE(status.ok()) << status;
  }
  reader = KzipReader::Open(absl::StrCat(TestTmpdir(), "/stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();
  std::unordered_map<std::string, std::unordered_set<std::string>> result;
  EXPECT_TRUE(
      reader
          ->Scan(WithStatus(
              &error,
              [&](absl::string_view digest) {
                if (auto unit = reader->ReadUnit(digest)) {
                  for (const auto& file : unit->unit().required_input()) {
                    if (auto data = reader->ReadFile(file.info().digest())) {
                      result[std::string(digest)].insert(file.info().digest());
                    } else {
                      return data.status();
                    }
                  }
                } else {
                  return unit.status();
                }
                return OkStatus();
              }))
          .ok());
  EXPECT_EQ(digests, result);
}

}  // namespace
}  // namespace kythe
