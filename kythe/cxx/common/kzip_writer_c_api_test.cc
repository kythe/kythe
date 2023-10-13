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

#include "kythe/cxx/common/kzip_writer_c_api.h"

#include <zip.h>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {
using ::testing::ElementsAre;
using ::testing::Values;

absl::string_view TestTmpdir() {
  return absl::StripSuffix(std::getenv("TEST_TMPDIR"), "/");
}

std::string TestOutputFile(absl::string_view basename) {
  const auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
  const auto filename =
      absl::StrReplaceAll(absl::StrCat(test_info->test_case_name(), "_",
                                       test_info->name(), "_", basename),
                          {{"/", "-"}});
  return absl::StrCat(TestTmpdir(), "/", filename);
}

TEST(KzipWriterCApiTest, CApiTest) {
  std::string dummy_file = TestOutputFile("dummy.kzip");
  int32_t status;
  std::unique_ptr<::KzipWriter, std::function<void(::KzipWriter*)>> w(
      KzipWriter_Create(dummy_file.c_str(), dummy_file.length(),
                        KZIP_WRITER_ENCODING_PROTO, &status),
      [](::KzipWriter* w) {
        if (w != nullptr) {
          KzipWriter_Delete(w);
        }
      });
  ASSERT_EQ(0, status);
  {
    std::string contents("contents");
    char digest[200];
    size_t length;
    int32_t status =
        KzipWriter_WriteFile(w.get(), contents.c_str(), contents.length(),
                             digest, sizeof(digest), &length);
    ASSERT_EQ(0, status);
  }
  {
    std::string proto_bytes;
    kythe::proto::IndexedCompilation unit;
    ASSERT_TRUE(unit.SerializeToString(&proto_bytes));
    char digest[800];
    size_t length;
    int32_t status =
        KzipWriter_WriteUnit(w.get(), proto_bytes.c_str(), proto_bytes.length(),
                             digest, sizeof(digest), &length);
    ASSERT_EQ(0, status);
  }
  {
    int32_t status = KzipWriter_Close(w.get());
    ASSERT_EQ(0, status);
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
          "root/", "root/files/", "root/pbunits/",
          "root/files/"
          "d1b2a59fbea7e20077af9f91b27e95e865061b270be03ff539ab3b73587882e8",
          "root/pbunits/"
          "44136fa355b3678a1146ad16f7e8649e94fb4fc21fe77e8310c060f61caaff8a"));
}

}  // namespace
}  // namespace kythe
