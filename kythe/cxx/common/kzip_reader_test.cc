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

#include "kythe/cxx/common/kzip_reader.h"

#include <stdlib.h>
#include <string>

#include "absl/base/port.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {

absl::string_view TestSrcdir() {
  return absl::StripSuffix(CHECK_NOTNULL(getenv("TEST_SRCDIR")), "/");
}

std::string TestFile(absl::string_view basename) {
  return absl::StrCat(TestSrcdir(), "/io_kythe/kythe/cxx/common/testdata/",
                      absl::StripPrefix(basename, "/"));
}

TEST(KzipReaderTest, OpenFailsForMissingFile) {
  EXPECT_EQ(KzipReader::Open(TestFile("MISSING.kzip")).status().code(),
            StatusCode::kNotFound);
}

TEST(KzipReaderTest, OpenFailsForEmptyFile) {
  EXPECT_EQ(KzipReader::Open(TestFile("empty.kzip")).status().code(),
            StatusCode::kInvalidArgument);
}

TEST(KzipReaderTest, OpenFailsForMissingRoot) {
  EXPECT_EQ(KzipReader::Open(TestFile("malformed.kzip")).status().code(),
            StatusCode::kInvalidArgument);
}

TEST(KzipReaderTest, OpenAndReadSimpleKzip) {
  StatusOr<IndexReader> reader = KzipReader::Open(TestFile("stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();
  EXPECT_TRUE(reader
                  ->Scan([&](absl::string_view digest) {
                    auto unit = reader->ReadUnit(digest);
                    if (unit.ok()) {
                      for (const auto& file : unit->unit().required_input()) {
                        auto data = reader->ReadFile(file.info().digest());
                        EXPECT_TRUE(data.ok())
                            << "Failed to read file contents: "
                            << data.status().ToString();
                        if (!data.ok()) {
                          return false;
                        }
                      }
                    }
                    EXPECT_TRUE(unit.ok())
                        << "Failed to read compilation unit: "
                        << unit.status().ToString();
                    return unit.ok();
                  })
                  .ok());
}
}  // namespace
}  // namespace kythe
