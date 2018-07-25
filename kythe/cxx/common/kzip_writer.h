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

#ifndef KYTHE_CXX_COMMON_KZIP_WRITER_H_
#define KYTHE_CXX_COMMON_KZIP_WRITER_H_

#include <zip.h>
#include <vector>

#include "kythe/cxx/common/index_writer.h"
#include "kythe/cxx/common/status_or.h"

namespace kythe {

class KzipWriter : public IndexWriterInterface {
 public:
  static StatusOr<IndexWriter> Create(absl::string_view path);
  static StatusOr<IndexWriter> FromSource(zip_source_t* source,
                                          int flags = ZIP_CREATE | ZIP_EXCL);

  ~KzipWriter() override;

  StatusOr<std::string> WriteUnit(
      const kythe::proto::IndexedCompilation& unit) override;
  StatusOr<std::string> WriteFile(absl::string_view content) override;
  Status Close() override;

 private:
  explicit KzipWriter(zip_t* archive);

  zip_t* archive_;  // Owned, but must be manually deleted via `Close`.
  std::vector<std::string>
      contents_;  // Memory for inserted files must be retained until close.
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KZIP_WRITER_H_
