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
#include <vector>

#include "absl/strings/escaping.h"
#include "glog/logging.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/kzip_encoding.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {

constexpr absl::string_view kRoot = "root/";
constexpr absl::string_view kJsonUnitRoot = "root/units/";
constexpr absl::string_view kProtoUnitRoot = "root/pbunits/";
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

absl::string_view Basename(absl::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == absl::string_view::npos) {
    return path;
  }
  return absl::ClippedSubstr(path, pos + 1);
}

bool HasEncoding(KzipEncoding lhs, KzipEncoding rhs) {
  return static_cast<typename std::underlying_type<KzipEncoding>::type>(lhs) &
         static_cast<typename std::underlying_type<KzipEncoding>::type>(rhs);
}

}  // namespace

/* static */
StatusOr<IndexWriter> KzipWriter::Create(absl::string_view path,
                                         KzipEncoding encoding) {
  int error;
  if (auto archive =
          zip_open(std::string(path).c_str(), ZIP_CREATE | ZIP_EXCL, &error)) {
    return IndexWriter(absl::WrapUnique(new KzipWriter(archive, encoding)));
  }
  return libzip::Error(error).ToStatus();
}

/* static */
StatusOr<IndexWriter> KzipWriter::FromSource(zip_source_t* source,
                                             KzipEncoding encoding,
                                             const int flags) {
  libzip::Error error;
  if (auto archive = zip_open_from_source(source, flags, error.get())) {
    return IndexWriter(absl::WrapUnique(new KzipWriter(archive, encoding)));
  }
  return error.ToStatus();
}

KzipWriter::KzipWriter(zip_t* archive, KzipEncoding encoding)
    : archive_(archive), encoding_(encoding) {}

KzipWriter::~KzipWriter() {
  DCHECK(archive_ == nullptr) << "Disposing of open KzipWriter!";
}

// Creates entries for the three directories if not already present.
Status KzipWriter::InitializeArchive(zip_t* archive) {
  std::vector<absl::string_view> dirs = {kRoot, kFileRoot};
  if (HasEncoding(encoding_, KzipEncoding::kJson)) {
    dirs.push_back(kJsonUnitRoot);
  }
  if (HasEncoding(encoding_, KzipEncoding::kProto)) {
    dirs.push_back(kProtoUnitRoot);
  }
  for (const auto& name : dirs) {
    if (zip_dir_add(archive, name.data(), ZIP_FL_ENC_UTF_8) < 0) {
      Status status = libzip::ToStatus(zip_get_error(archive));
      zip_error_clear(archive);
      return status;
    }
  }
  return OkStatus();
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
  auto json = WriteMessageAsJsonToString(unit);
  if (!json) {
    return json.status();
  }
  auto digest = SHA256Digest(*json);
  StatusOr<std::string> result = InternalError("unsupported encoding");;
  if (HasEncoding(encoding_, KzipEncoding::kJson)) {
    result = InsertFile(absl::StrCat(kJsonUnitRoot, digest), *json);
    if (!result) {
      return result;
    }
  }
  if (HasEncoding(encoding_, KzipEncoding::kProto)) {
    std::string contents;
    if (!unit.SerializeToString(&contents)) {
      return InternalError("Failure serializing compilation unit");
    }
    result = InsertFile(absl::StrCat(kProtoUnitRoot, digest), contents);
  }
  return result;
}

StatusOr<std::string> KzipWriter::WriteFile(absl::string_view content) {
  if (!initialized_) {
    auto status = InitializeArchive(archive_);
    if (!status.ok()) {
      return status;
    }
    initialized_ = true;
  }
  return InsertFile(absl::StrCat(kFileRoot, SHA256Digest(content)), content);
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

StatusOr<std::string> KzipWriter::InsertFile(absl::string_view path, absl::string_view content) {
  // Initially insert an empty string for the file content.
  auto insertion = contents_.emplace(path, "");
  if (insertion.second) {
    // Only copy in the real content if it was actually inserted into the map.
    auto& entry = insertion.first;
    entry->second = std::string(content);
    auto status = WriteTextFile(archive_, entry->first, entry->second);
    if (!status.ok()) {
      contents_.erase(entry->first);
      return status;
    }
  }
  return std::string(Basename(path));
}

}  // namespace kythe
