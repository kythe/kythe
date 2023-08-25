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

#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <string>
#include <utility>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/common/index_writer.h"
#include "kythe/cxx/common/kzip_encoding.h"
#include "kythe/cxx/common/kzip_writer.h"
#include "kythe/proto/analysis.pb.h"

using kythe::IndexWriter;
using kythe::KzipEncoding;

namespace {

KzipEncoding ToKzipEncoding(int32_t encoding) {
  KzipEncoding kzip_encoding = KzipEncoding::kJson;
  switch (encoding) {
    case KZIP_WRITER_ENCODING_JSON:
      kzip_encoding = KzipEncoding::kJson;
      break;
    case KZIP_WRITER_ENCODING_PROTO:
      kzip_encoding = KzipEncoding::kProto;
      break;
    default:
      // fallthrough
    case KZIP_WRITER_ENCODING_ALL:
      kzip_encoding = kythe::KzipEncoding::kAll;
      break;
  }
  return kzip_encoding;
}

int32_t CodeFromStatus(const absl::Status& s) {
  const absl::StatusCode code = s.code();
  LOG(ERROR) << "Status: " << s;
  return static_cast<int32_t>(code);
}

}  // namespace

KzipWriter* KzipWriter_Create(const char* path, const size_t path_len,
                              int32_t encoding, int32_t* create_status) {
  CHECK(path != nullptr);
  *create_status = 0;
  KzipEncoding kzip_encoding = ToKzipEncoding(encoding);
  const absl::string_view path_view(path, path_len);

  absl::StatusOr<IndexWriter> writer =
      kythe::KzipWriter::Create(path_view, kzip_encoding);
  if (!writer.ok()) {
    *create_status = CodeFromStatus(writer.status());
    return nullptr;
  }
  return reinterpret_cast<KzipWriter*>(
      new kythe::IndexWriter(*std::move(writer)));
}

void KzipWriter_Delete(KzipWriter* writer) {
  // We assume that `writer` was created by `KzipWriter_Create`.
  delete reinterpret_cast<IndexWriter*>(writer);
}

int32_t KzipWriter_Close(KzipWriter* writer) {
  const absl::Status status = reinterpret_cast<IndexWriter*>(writer)->Close();
  return static_cast<int32_t>(status.code());
}

int32_t KzipWriter_WriteFile(KzipWriter* writer, const char* content,
                             const size_t content_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size) {
  // We assume that `writer` was created by `KzipWriter_Create`.
  IndexWriter* w = reinterpret_cast<IndexWriter*>(writer);
  auto digest_or = w->WriteFile({content, content_length});
  if (!digest_or.ok()) {
    return CodeFromStatus(digest_or.status());
  }
  const std::string& digest = *digest_or;
  if (digest.length() > buffer_length) {
    LOG(ERROR) << "Digest buffer too small to fit digest: " << digest;
    return KZIP_WRITER_BUFFER_TOO_SMALL_ERROR;
  }
  memcpy(digest_buffer, digest.c_str(), digest.length());
  *resulting_digest_size = digest.length();
  return 0;
}

int32_t KzipWriter_WriteUnit(KzipWriter* writer, const char* proto,
                             size_t proto_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size) {
  // We assume that `writer` was created by `KzipWriter_Create`.
  IndexWriter* w = reinterpret_cast<IndexWriter*>(writer);
  kythe::proto::IndexedCompilation unit;
  const bool success = unit.ParseFromArray(proto, proto_length);
  if (!success) {
    LOG(ERROR) << "Protobuf could not be parsed, at len: " << proto_length;
    return KZIP_WRITER_PROTO_PARSING_ERROR;
  }
  absl::StatusOr<std::string> digest_or = w->WriteUnit(unit);
  if (!digest_or.ok()) {
    return CodeFromStatus(digest_or.status());
  }
  const auto& digest = *digest_or;
  if (digest.length() > buffer_length) {
    return KZIP_WRITER_BUFFER_TOO_SMALL_ERROR;
  }
  memcpy(digest_buffer, digest.c_str(), digest.length());
  *resulting_digest_size = digest.length();
  return 0;
}
