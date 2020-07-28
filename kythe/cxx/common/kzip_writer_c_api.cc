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

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/common/index_writer.h"
#include "kythe/cxx/common/kzip_writer.h"
#include "kythe/cxx/common/status_or.h"
#include "kythe/proto/analysis.pb.h"

using kythe::IndexWriter;
using kythe::KzipEncoding;
using kythe::StatusOr;

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
  return static_cast<int32_t>(code);
}

}  // namespace

KzipWriter* KzipWriter_Create(const char* path, const size_t path_len,
                              int32_t encoding, int32_t* create_status) {
  *create_status = 0;
  KzipEncoding kzip_encoding = ToKzipEncoding(encoding);
  const absl::string_view path_view(path, path_len);

  StatusOr<std::unique_ptr<IndexWriter>> writer_or =
      kythe::KzipWriter::CreateUnique(path_view, kzip_encoding);
  if (!writer_or.ok()) {
    *create_status = CodeFromStatus(writer_or.status());
    return nullptr;
  }
  std::unique_ptr<IndexWriter>& writer = *writer_or;
  KzipWriter* writer_ptr = reinterpret_cast<KzipWriter*>(writer.get());
  writer.release();
  return writer_ptr;
}

void KzipWriter_Delete(KzipWriter* writer) {
  // We assume that `writer` was created by `KzipWriter_Create`.
  std::unique_ptr<IndexWriter> _delete(reinterpret_cast<IndexWriter*>(writer));
}

int32_t KzipWriter_Close(KzipWriter* writer) {
  IndexWriter* w = reinterpret_cast<IndexWriter*>(writer);
  absl::Status s = w->Close();
  return static_cast<int32_t>(s.code());
}

int32_t KzipWriter_WriteFile(KzipWriter* writer, const char* content,
                             const size_t content_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size) {
  // We assume that `writer` was created by `KzipWriter_Create`.
  IndexWriter* w = reinterpret_cast<IndexWriter*>(writer);
  absl::string_view view(content, content_length);
  auto digest_or = w->WriteFile(content);
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

int32_t KzipWriter_WriteUnit(KzipWriter* writer, const char* proto,
                             size_t proto_length, char* digest_buffer,
                             size_t buffer_length,
                             size_t* resulting_digest_size) {
  //// We assume that `writer` was created by `KzipWriter_Create`.
  IndexWriter* w = reinterpret_cast<IndexWriter*>(writer);
  const std::string_view proto_bytes(proto, proto_length);
  kythe::proto::IndexedCompilation unit;
  const bool success = unit.ParseFromString(std::string(proto_bytes));
  if (!success) {
    return KZIP_WRITER_PROTO_PARSING_ERROR;
  }
  auto digest_or = w->WriteUnit(unit);
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
