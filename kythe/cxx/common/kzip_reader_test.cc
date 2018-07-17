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
  StatusOr<std::unique_ptr<IndexReaderInterface>> reader =
      KzipReader::Open(TestFile("stringset.kzip"));
  ASSERT_TRUE(reader.ok()) << reader.status();
  EXPECT_TRUE((*reader)->Scan([&](absl::string_view digest) {
    if(auto unit = (*reader)->ReadUnit(digest)) {
    } else {
      EXPECT_TRUE(unit.ok())
          << "Failed to read compilation unit: " << unit.status().ToString();
    }
    return true;
  }).ok());
  ASSERT_TRUE(false);
}

}  // namespace
}  // namespace kythe
