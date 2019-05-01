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

#include <unordered_map>

#include "absl/strings/string_view.h"
#include "kythe/cxx/common/index_writer.h"
#include "kythe/cxx/common/status_or.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief Kzip implementation of IndexWriter.
/// see https://www.kythe.io/docs/kythe-kzip.html for format description.
class KzipWriter : public IndexWriterInterface {
 public:
  /// \brief Constructs a Kzip IndexWriter which will create and write to
  /// \param path Path to the file to create. Must not currently exist.
  static StatusOr<IndexWriter> Create(absl::string_view path);
  /// \brief Constructs an IndexWriter from the libzip source pointer.
  /// \param source zip_source_t to use as backing store.
  /// See https://libzip.org/documentation/zip_source.html for ownership.
  /// \param flags Flags to use when opening `source`.
  static StatusOr<IndexWriter> FromSource(zip_source_t* source,
                                          int flags = ZIP_CREATE | ZIP_EXCL);

  /// \brief Destroys the KzipWriter.
  ~KzipWriter() override;

  /// \brief Writes the unit to the kzip file, returning its digest.
  StatusOr<std::string> WriteUnit(
      const kythe::proto::IndexedCompilation& unit) override;

  /// \brief Writes the file contents to the kzip file, returning their digest.
  StatusOr<std::string> WriteFile(absl::string_view content) override;

  /// \brief Flushes accumulated writes and closes the kzip file.
  /// Close must be called before the KzipWriter is destroyed!
  Status Close() override;

 private:
  using Path = std::string;
  using Contents = std::string;
  using FileMap = std::unordered_map<Path, Contents>;

  struct InsertionResult {
    absl::string_view digest() const;
    const std::string& path() const { return insertion.first->first; }
    absl::string_view contents() const { return insertion.first->second; }
    bool inserted() const { return insertion.second; }

    std::pair<FileMap::iterator, bool> insertion;
  };

  explicit KzipWriter(zip_t* archive);

  InsertionResult InsertFile(absl::string_view root, absl::string_view content);

  bool initialized_ = false;  // Whether or not the `root` entry exists.
  zip_t* archive_;  // Owned, but must be manually deleted via `Close`.
  // Memory for inserted files must be retained until close and
  // we don't want to insert identical entries multiple times.
  // This must be a node-based container to ensure pointer stability of the file
  // contents.
  FileMap contents_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KZIP_WRITER_H_
