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

#include <zip.h>
#include <zipconf.h>

#include <functional>
#include <string>

#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/index_reader.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/cxx/common/testutil.h"
#include "kythe/proto/go.pb.h"

namespace kythe {
namespace {

using absl::StatusCode;

/// \brief Returns an error for any command except ZIP_SOURCE_ERROR.
zip_int64_t BadZipSource(void* state, void* data, zip_uint64_t len,
                         zip_source_cmd_t cmd) {
  switch (cmd) {
    case ZIP_SOURCE_FREE:
      return 0;
    case ZIP_SOURCE_ERROR:
      return zip_error_to_data(static_cast<zip_error_t*>(state), data, len);
    default:
      zip_error_set(static_cast<zip_error_t*>(state), ZIP_ER_INVAL, 0);
      return -1;
  }
}

using ZipCallback =
    std::function<zip_int64_t(void*, zip_uint64_t, zip_source_cmd_t)>;

/// \brief Trampoline that forwards to a std::function.
zip_int64_t FunctionTrampoline(void* state, void* data, zip_uint64_t len,
                               zip_source_cmd_t cmd) {
  return (*static_cast<ZipCallback*>(state))(data, len, cmd);
}

zip_source_t* ZipSourceFunctionCreate(ZipCallback* callback,
                                      zip_error_t* error) {
  return zip_source_function_create(FunctionTrampoline,
                                    static_cast<void*>(callback), error);
}

std::string TestFile(absl::string_view basename) {
  return absl::StrCat(TestSourceRoot(), "kythe/testdata/platform/",
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

TEST(KzipReaderTest, OpenFailsForMissingPbUnit) {
  EXPECT_EQ(KzipReader::Open(TestFile("missing-pbunit.kzip")).status().code(),
            StatusCode::kInvalidArgument);
}

TEST(KzipReaderTest, OpenFailsForMissingUnit) {
  EXPECT_EQ(KzipReader::Open(TestFile("missing-unit.kzip")).status().code(),
            StatusCode::kInvalidArgument);
}

TEST(KzipReaderTest, OpenFailsForMissingRoot) {
  EXPECT_EQ(KzipReader::Open(TestFile("malformed.kzip")).status().code(),
            StatusCode::kInvalidArgument);
}

TEST(KzipReaderTest, OpenAndReadSimpleKzip) {
  // This forces the GoDetails proto descriptor to be added to the pool so we
  // can deserialize it. If we don't do this, we get an error like:
  // "Invalid type URL, unknown type: kythe.proto.GoDetails for type Any".
  proto::GoDetails needed_for_proto_deserialization;

  absl::StatusOr<IndexReader> reader =
      KzipReader::Open(TestFile("stringset.kzip"));
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

TEST(KzipReaderTest, FromSourceFailsIfSourceDoes) {
  libzip::Error error;
  {
    libzip::Error inner;

    zip_source_t* source = zip_source_function_create(
        BadZipSource, static_cast<void*>(error.get()), inner.get());
    EXPECT_EQ(KzipReader::FromSource(source).status().code(),
              StatusCode::kUnimplemented);
    zip_source_free(source);
  }
}

// Checks that when there is an error due to a zero-length
// file, the reference count is maintained correctly.
TEST(KzipReaderTest, SourceFreedOnEmptyFile) {
  bool freed = false;
  zip_error_t error;
  ZipCallback callback = [&](void* data, zip_uint64_t len,
                             zip_source_cmd_t cmd) -> zip_int64_t {
    switch (cmd) {
      case ZIP_SOURCE_FREE:
        freed = true;
        return 0;
      case ZIP_SOURCE_STAT:
        if (zip_stat_t* stat =
                ZIP_SOURCE_GET_ARGS(zip_stat_t, data, len, &error)) {
          stat->size = 0;
          stat->valid |= ZIP_STAT_SIZE;
          return sizeof(zip_stat_t);
        } else {
          return -1;
        }
      case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&error, data, len);
      case ZIP_SOURCE_SUPPORTS:
        // KzipReader requires SEEKABLE.
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
      default:
        LOG(ERROR) << "Unsupported operation: " << cmd;
        zip_error_set(&error, ZIP_ER_INVAL, 0);
        return -1;
    }
  };
  {
    zip_source_t* source = ZipSourceFunctionCreate(&callback, nullptr);
    ASSERT_NE(source, nullptr);
    absl::StatusOr<IndexReader> reader = KzipReader::FromSource(source);
    ASSERT_EQ(reader.status().code(), StatusCode::kInvalidArgument);
    ASSERT_FALSE(freed);
    zip_source_free(source);
    EXPECT_TRUE(freed);
  }
}

// Checks that when there is an error due to an invalid zip
// file, the reference count is maintained correctly.
TEST(KzipReaderTest, SourceFreedOnInvalidFile) {
  bool freed = false;
  const absl::string_view buf = "this is not a valid zip file";
  zip_source_t* buf_src =
      zip_source_buffer_create(buf.data(), buf.size(), false, nullptr);
  ASSERT_NE(buf_src, nullptr);

  ZipCallback callback = [&](void* data, zip_uint64_t len,
                             zip_source_cmd_t cmd) -> zip_int64_t {
    switch (cmd) {
      case ZIP_SOURCE_OPEN:
        return zip_source_open(buf_src);
      case ZIP_SOURCE_READ:
        return zip_source_read(buf_src, data, len);
      case ZIP_SOURCE_CLOSE:
        return zip_source_close(buf_src);
      case ZIP_SOURCE_STAT:
        if (zip_stat_t* stat =
                ZIP_SOURCE_GET_ARGS(zip_stat_t, data, len, nullptr)) {
          return zip_source_stat(buf_src, stat) == 0 ? sizeof(zip_stat_t) : -1;
        } else {
          return -1;
        }
      case ZIP_SOURCE_ERROR:
        return zip_error_to_data(zip_source_error(buf_src), data, len);
      case ZIP_SOURCE_FREE:
        freed = true;
        zip_source_free(buf_src);
        return 0;
      case ZIP_SOURCE_SEEK:
        if (zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(
                zip_source_args_seek_t, data, len, nullptr)) {
          return zip_source_seek(buf_src, args->offset, args->whence);
        }
        return -1;
      case ZIP_SOURCE_TELL:
        return zip_source_tell(buf_src);
      case ZIP_SOURCE_BEGIN_WRITE:
        return zip_source_begin_write(buf_src);
      case ZIP_SOURCE_COMMIT_WRITE:
        return zip_source_commit_write(buf_src);
      case ZIP_SOURCE_ROLLBACK_WRITE:
        zip_source_rollback_write(buf_src);
        return 0;
      case ZIP_SOURCE_WRITE:
        return zip_source_write(buf_src, data, len);
      case ZIP_SOURCE_SEEK_WRITE:
        if (zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(
                zip_source_args_seek_t, data, len, nullptr)) {
          return zip_source_seek_write(buf_src, args->offset, args->whence);
        }
        return -1;
      case ZIP_SOURCE_TELL_WRITE:
        return zip_source_tell_write(buf_src);
      case ZIP_SOURCE_SUPPORTS:
        // Cannot wrap this either.
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
      case ZIP_SOURCE_REMOVE:
        // No way to wrap this call.
        return 0;
      default:
        return -1;
    }
  };
  {
    zip_source_t* source = ZipSourceFunctionCreate(&callback, nullptr);
    ASSERT_NE(source, nullptr);
    absl::StatusOr<IndexReader> reader = KzipReader::FromSource(source);
    ASSERT_EQ(reader.status().code(), StatusCode::kInvalidArgument);
    ASSERT_FALSE(freed);
    zip_source_free(source);
    EXPECT_TRUE(freed);
  }
}

// Checks that when there is an error due to a violation
// of kzip-specific requirements the reference count is
// maintained correctly.
TEST(KzipReaderTest, SourceFreedOnEmptyZipFile) {
  // A valid zip file, but an invalid Kzip file.
  const char buffer[] =
      "PK\x05\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00";
  zip_source_t* buf_src =
      zip_source_buffer_create(buffer, sizeof(buffer), false, nullptr);
  ASSERT_NE(buf_src, nullptr);
  bool freed = false;
  ZipCallback callback = [&](void* data, zip_uint64_t len,
                             zip_source_cmd_t cmd) -> zip_int64_t {
    switch (cmd) {
      case ZIP_SOURCE_OPEN:
        return zip_source_open(buf_src);
      case ZIP_SOURCE_READ:
        return zip_source_read(buf_src, data, len);
      case ZIP_SOURCE_CLOSE:
        return zip_source_close(buf_src);
      case ZIP_SOURCE_STAT:
        if (zip_stat_t* stat =
                ZIP_SOURCE_GET_ARGS(zip_stat_t, data, len, nullptr)) {
          return zip_source_stat(buf_src, stat) == 0 ? sizeof(zip_stat_t) : -1;
        } else {
          return -1;
        }
      case ZIP_SOURCE_ERROR:
        return zip_error_to_data(zip_source_error(buf_src), data, len);
      case ZIP_SOURCE_FREE:
        freed = true;
        zip_source_free(buf_src);
        return 0;
      case ZIP_SOURCE_SEEK:
        if (zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(
                zip_source_args_seek_t, data, len, nullptr)) {
          return zip_source_seek(buf_src, args->offset, args->whence);
        }
        return -1;
      case ZIP_SOURCE_TELL:
        return zip_source_tell(buf_src);
      case ZIP_SOURCE_BEGIN_WRITE:
        return zip_source_begin_write(buf_src);
      case ZIP_SOURCE_COMMIT_WRITE:
        return zip_source_commit_write(buf_src);
      case ZIP_SOURCE_ROLLBACK_WRITE:
        zip_source_rollback_write(buf_src);
        return 0;
      case ZIP_SOURCE_WRITE:
        return zip_source_write(buf_src, data, len);
      case ZIP_SOURCE_SEEK_WRITE:
        if (zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(
                zip_source_args_seek_t, data, len, nullptr)) {
          return zip_source_seek_write(buf_src, args->offset, args->whence);
        }
        return -1;
      case ZIP_SOURCE_TELL_WRITE:
        return zip_source_tell_write(buf_src);
      case ZIP_SOURCE_SUPPORTS:
        // Cannot wrap this either.
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
      case ZIP_SOURCE_REMOVE:
        // No way to wrap this call.
        return 0;
      default:
        return -1;
    }
  };
  {
    zip_source_t* source = ZipSourceFunctionCreate(&callback, nullptr);
    ASSERT_NE(source, nullptr);
    absl::StatusOr<IndexReader> reader = KzipReader::FromSource(source);
    ASSERT_EQ(reader.status().code(), StatusCode::kInvalidArgument);
    ASSERT_FALSE(freed);
    zip_source_free(source);
    EXPECT_TRUE(freed);
  }
}

TEST(KzipReaderTest, FromSourceReadsSimpleKzip) {
  // This forces the GoDetails proto descriptor to be added to the pool so we
  // can deserialize it. If we don't do this, we get an error like:
  // "Invalid type URL, unknown type: kythe.proto.GoDetails for type Any".
  proto::GoDetails needed_for_proto_deserialization;

  libzip::Error error;
  absl::StatusOr<IndexReader> reader =
      KzipReader::FromSource(zip_source_file_create(
          TestFile("stringset.kzip").c_str(), 0, -1, error.get()));

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
