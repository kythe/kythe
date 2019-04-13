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

#include <openssl/sha.h>

#include <array>
#include <string>
#include <tuple>

#include "absl/strings/escaping.h"
#include "glog/logging.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {

constexpr absl::string_view kRoot = "root/";
constexpr absl::string_view kUnitRoot = "root/units/";
constexpr absl::string_view kFileRoot = "root/files/";

std::string SHA256Digest(absl::string_view content) {
  std::array<unsigned char, SHA256_DIGEST_LENGTH> buf;
  ::SHA256(reinterpret_cast<const unsigned char*>(content.data()),
           content.size(), buf.data());
  return absl::BytesToHexString(
      absl::string_view(reinterpret_cast<const char*>(buf.data()), buf.size()));
}

Status WriteTextFile(zip_t* archive, const std::string& path,
                     absl::string_view content) {
  if (auto source =
          zip_source_buffer(archive, content.data(), content.size(), 0)) {
    if (zip_file_add(archive, path.c_str(), source, ZIP_FL_ENC_UTF_8) >= 0) {
      return OkStatus();
    }
    zip_source_free(source);
  }
  return libzip::ToStatus(zip_get_error(archive));
}

// Creates entries for the three directories if not already present.
Status InitializeArchive(zip_t* archive) {
  for (const auto name : {kRoot, kUnitRoot, kFileRoot}) {
    if (zip_dir_add(archive, name.data(), ZIP_FL_ENC_UTF_8) < 0) {
      Status status = libzip::ToStatus(zip_get_error(archive));
      zip_error_clear(archive);
      return status;
    }
  }
  return OkStatus();
}

absl::string_view Basename(absl::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == absl::string_view::npos) {
    return path;
  }
  return absl::ClippedSubstr(path, pos + 1);
}

}  // namespace

/* static */
StatusOr<IndexWriter> KzipWriter::Create(absl::string_view path) {
  int error;
  if (auto archive =
          zip_open(std::string(path).c_str(), ZIP_CREATE | ZIP_EXCL, &error)) {
    return IndexWriter(absl::WrapUnique(new KzipWriter(archive)));
  }
  return libzip::Error(error).ToStatus();
}

/* static */
StatusOr<IndexWriter> KzipWriter::FromSource(zip_source_t* source,
                                             const int flags) {
  libzip::Error error;
  if (auto archive = zip_open_from_source(source, flags, error.get())) {
    return IndexWriter(absl::WrapUnique(new KzipWriter(archive)));
  }
  return error.ToStatus();
}

KzipWriter::KzipWriter(zip_t* archive) : archive_(archive) {}

KzipWriter::~KzipWriter() {
  DCHECK(archive_ == nullptr) << "Disposing of open KzipWriter!";
}

StatusOr<std::string> KzipWriter::WriteUnit(
    const kythe::proto::IndexedCompilation& unit) {
  if (!initialized_) {
    auto status = InitializeArchive(archive_);
    if (!status.ok()) {
      return status;
    }
    initialized_ = true;
  }
  if (auto json = WriteMessageAsJsonToString(unit)) {
    auto file = InsertFile(kUnitRoot, *json);
    if (file.inserted()) {
      auto status = WriteTextFile(archive_, file.path(), file.contents());
      if (!status.ok()) {
        contents_.erase(file.path());
        return status;
      }
    }
    return std::string(file.digest());
  } else {
    return json.status();
  }
}

StatusOr<std::string> KzipWriter::WriteFile(absl::string_view content) {
  if (!initialized_) {
    auto status = InitializeArchive(archive_);
    if (!status.ok()) {
      return status;
    }
    initialized_ = true;
  }
  auto file = InsertFile(kFileRoot, content);
  if (file.inserted()) {
    auto status = WriteTextFile(archive_, file.path(), file.contents());
    if (!status.ok()) {
      contents_.erase(file.path());
      return status;
    }
  }
  return std::string(file.digest());
}

Status KzipWriter::Close() {
  DCHECK(archive_ != nullptr);

  Status result = OkStatus();
  if (zip_close(archive_) != 0) {
    result = libzip::ToStatus(zip_get_error(archive_));
    zip_discard(archive_);
  }

  archive_ = nullptr;
  contents_.clear();
  return result;
}

auto KzipWriter::InsertFile(absl::string_view root, absl::string_view content)
    -> InsertionResult {
  auto digest = SHA256Digest(content);
  auto path = absl::StrCat(root, digest);
  // Initially insert an empty string for the file content.
  auto result = InsertionResult{contents_.emplace(path, "")};
  if (result.inserted()) {
    // Only copy in the real content if it was actually inserted into the map.
    result.insertion.first->second = std::string(content);
  }
  return result;
}

inline absl::string_view KzipWriter::InsertionResult::digest() const {
  return Basename(path());
}

}  // namespace kythe
