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

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "kythe/cxx/common/index_reader.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/kzip_encoding.h"
#include "kythe/cxx/common/libzip/error.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace {

constexpr absl::string_view kJsonUnitsDir = "/units/";
constexpr absl::string_view kProtoUnitsDir = "/pbunits/";

struct ZipFileClose {
  void operator()(zip_file_t* file) {
    if (file != nullptr) {
      CHECK_EQ(zip_fclose(file), 0);
    }
  }
};
using ZipFile = std::unique_ptr<zip_file_t, ZipFileClose>;

class ZipFileInputStream : public google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit ZipFileInputStream(zip_file_t* file) : input_(file) {}

  bool Next(const void** data, int* size) override {
    return impl_.Next(data, size);
  }

  void BackUp(int count) override { impl_.BackUp(count); }
  bool Skip(int count) override { return impl_.Skip(count); }
  int64_t ByteCount() const override { return impl_.ByteCount(); }

 private:
  class CopyingZipInputStream
      : public google::protobuf::io::CopyingInputStream {
   public:
    explicit CopyingZipInputStream(zip_file_t* file) : file_(file) {}

    int Read(void* buffer, int size) override {
      return zip_fread(file_, buffer, size);
    }

    int Skip(int count) override {
      zip_int64_t start = zip_ftell(file_);
      if (start < 0) {
        return 0;
      }
      if (zip_fseek(file_, count, SEEK_CUR) < 0) {
        return 0;
      }
      zip_int64_t end = zip_ftell(file_);
      if (end < 0) {
        return 0;
      }
      return end - start;
    }

   private:
    zip_file_t* file_;
  };

  CopyingZipInputStream input_;
  google::protobuf::io::CopyingInputStreamAdaptor impl_{&input_};
};

struct KzipOptions {
  absl::string_view root;
  KzipEncoding encoding;
};

absl::StatusOr<KzipOptions> Validate(zip_t* archive) {
  if (!zip_get_num_entries(archive, 0)) {
    return absl::InvalidArgumentError("Empty kzip archive");
  }

  // Pull the root directory from an arbitrary entry.
  absl::string_view root = zip_get_name(archive, 0, 0);
  auto slashpos = root.find('/');
  if (slashpos == 0 || slashpos == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("Malformed kzip: invalid root: ", root));
  }
  root.remove_suffix(root.size() - slashpos);
  DLOG(LEVEL(-1)) << "Using archive root: " << root;
  std::set<absl::string_view> proto_units;
  std::set<absl::string_view> json_units;
  for (int i = 0; i < zip_get_num_entries(archive, 0); ++i) {
    absl::string_view name = zip_get_name(archive, i, 0);
    if (!absl::ConsumePrefix(&name, root)) {
      return absl::InvalidArgumentError(
          absl::StrCat("Malformed kzip: invalid entry: ", name));
    }
    if (absl::ConsumePrefix(&name, kJsonUnitsDir)) {
      json_units.insert(name);
    } else if (absl::ConsumePrefix(&name, kProtoUnitsDir)) {
      proto_units.insert(name);
    }
  }
  KzipEncoding encoding = KzipEncoding::kJson;
  if (json_units.empty()) {
    encoding = KzipEncoding::kProto;
  } else if (!proto_units.empty()) {
    std::vector<absl::string_view> diff;
    std::set_symmetric_difference(json_units.begin(), json_units.end(),
                                  proto_units.begin(), proto_units.end(),
                                  std::inserter(diff, diff.end()));
    if (!diff.empty()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Malformed kzip: multiple unit encodings but different entries"));
    }
  }
  return KzipOptions{root, encoding};
}

std::optional<zip_uint64_t> FileSize(zip_t* archive, zip_uint64_t index) {
  zip_stat_t sb;
  zip_stat_init(&sb);

  if (zip_stat_index(archive, index, ZIP_STAT_SIZE, &sb) < 0) {
    return std::nullopt;
  }
  return sb.size;
}

absl::StatusOr<std::string> ReadTextFile(zip_t* archive,
                                         const std::string& path) {
  zip_int64_t index = zip_name_locate(archive, path.c_str(), 0);
  if (index >= 0) {
    if (auto file = ZipFile(zip_fopen_index(archive, index, 0))) {
      if (auto size = FileSize(archive, index)) {
        std::string result(*size, '\0');
        if (*size == 0 ||
            zip_fread(file.get(), result.data(), *size) == *size) {
          return result;
        } else {
          return libzip::ToStatus(zip_file_get_error(file.get()));
        }
      }
    }
  }
  absl::Status status = libzip::ToStatus(zip_get_error(archive));
  if (!status.ok()) {
    return status;
  }
  return absl::UnknownError(absl::StrCat("Unable to read: ", path));
}

absl::string_view DirNameForEncoding(KzipEncoding encoding) {
  switch (encoding) {
    case KzipEncoding::kJson:
      return kJsonUnitsDir;
    case KzipEncoding::kProto:
      return kProtoUnitsDir;
    default:
      LOG(FATAL) << "Unsupported encoding: " << static_cast<int>(encoding);
  }
  return "";
}

}  // namespace

std::optional<absl::string_view> KzipReader::UnitDigest(
    absl::string_view path) {
  if (!absl::ConsumePrefix(&path, unit_prefix_) || path.empty()) {
    return std::nullopt;
  }
  return path;
}

/* static */
absl::StatusOr<IndexReader> KzipReader::Open(absl::string_view path) {
  int error;
  if (auto archive =
          ZipHandle(zip_open(std::string(path).c_str(), ZIP_RDONLY, &error))) {
    if (auto options = Validate(archive.get()); options.ok()) {
      return IndexReader(absl::WrapUnique(new KzipReader(
          std::move(archive), options->root, options->encoding)));
    } else {
      return options.status();
    }
  }
  return libzip::Error(error).ToStatus();
}

/* static */
absl::StatusOr<IndexReader> KzipReader::FromSource(zip_source_t* source) {
  libzip::Error error;
  if (auto archive =
          ZipHandle(zip_open_from_source(source, ZIP_RDONLY, error.get()))) {
    if (auto options = Validate(archive.get()); options.ok()) {
      return IndexReader(absl::WrapUnique(new KzipReader(
          std::move(archive), options->root, options->encoding)));
    } else {
      // Ensure source is retained when `archive` is deleted.
      // It is the callers responsitility to free it on error.
      zip_source_keep(source);
      return options.status();
    }
  }
  return error.ToStatus();
}

KzipReader::KzipReader(ZipHandle archive, absl::string_view root,
                       KzipEncoding encoding)
    : archive_(std::move(archive)),
      encoding_(encoding),
      files_prefix_(absl::StrCat(root, "/files/")),
      unit_prefix_(absl::StrCat(root, DirNameForEncoding(encoding))) {}

absl::StatusOr<proto::IndexedCompilation> KzipReader::ReadUnit(
    absl::string_view digest) {
  std::string path = absl::StrCat(unit_prefix_, digest);

  if (auto file = ZipFile(zip_fopen(archive(), path.c_str(), 0))) {
    proto::IndexedCompilation unit;
    ZipFileInputStream input(file.get());
    absl::Status status;
    if (encoding_ == KzipEncoding::kJson) {
      status = ParseFromJsonStream(&input, &unit);
    } else {
      if (!unit.ParseFromZeroCopyStream(&input)) {
        status = absl::InvalidArgumentError("Failure parsing proto unit");
      }
    }
    if (!status.ok()) {
      absl::Status zip_status =
          libzip::ToStatus(zip_file_get_error(file.get()));
      if (!zip_status.ok()) {
        // Prefer the underlying zip error, if present.
        return zip_status;
      }
      return status;
    }
    return unit;
  }
  absl::Status status = libzip::ToStatus(zip_get_error(archive()));
  if (!status.ok()) {
    return status;
  }
  return absl::UnknownError(absl::StrCat("Unable to open unit ", digest));
}

absl::StatusOr<std::string> KzipReader::ReadFile(absl::string_view digest) {
  return ReadTextFile(archive(), absl::StrCat(files_prefix_, digest));
}

absl::Status KzipReader::Scan(const ScanCallback& callback) {
  for (int i = 0; i < zip_get_num_entries(archive(), 0); ++i) {
    if (auto digest = UnitDigest(zip_get_name(archive(), i, 0))) {
      if (!callback(*digest)) {
        break;
      }
    }
  }
  return absl::OkStatus();
}

}  // namespace kythe
