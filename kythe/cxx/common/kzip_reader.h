/*
 * Copyright 2018 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_KZIP_READER_H_
#define KYTHE_CXX_COMMON_KZIP_READER_H_

#include <functional>
#include <string>

#include <zip.h>

#include "absl/strings/string_view.h"
#include "kythe/cxx/common/index_reader.h"
#include "kythe/cxx/common/status_or.h"

namespace kythe {

class KzipReader : public IndexReaderInterface {
 public:
  static StatusOr<IndexReader> Open(absl::string_view path);

  Status Scan(const ScanCallback& callback) override;

  StatusOr<kythe::proto::IndexedCompilation> ReadUnit(
      absl::string_view digest) override;

  StatusOr<std::string> ReadFile(absl::string_view digest) override;

 private:
  struct Discard {
    void operator()(zip_t* archive) {
      if (archive) zip_discard(archive);
    }
  };
  using ZipHandle = std::unique_ptr<zip_t, Discard>;

  explicit KzipReader(ZipHandle archive, absl::string_view basename);

  zip_t* archive() { return archive_.get(); }

  ZipHandle archive_;
  absl::string_view root_; // Memory owned by `archive_`.
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KZIP_READER_H_
