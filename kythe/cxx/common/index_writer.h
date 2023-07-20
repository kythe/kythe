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

#ifndef KYTHE_CXX_COMMON_INDEX_WRITER_H_
#define KYTHE_CXX_COMMON_INDEX_WRITER_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include <string_view>
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief Simple interface for writing IndexedCompilations and files
/// to an underlying data store.
class IndexWriterInterface {
 public:
  IndexWriterInterface() = default;
  // IndexWriterInterface is neither copyable nor movable.
  IndexWriterInterface(const IndexWriterInterface&) = delete;
  IndexWriterInterface& operator=(const IndexWriterInterface&) = delete;
  virtual ~IndexWriterInterface() = default;

  /// \brief Write the `IndexedCompilation` to the index. Returns the
  /// hex-encoded SHA256 digest of the unit's contents.
  virtual absl::StatusOr<std::string> WriteUnit(
      const kythe::proto::IndexedCompilation& unit) = 0;

  /// \brief Write the file data to the index. Returns the hex-encoded
  /// SHA256 digest of the file's contents.
  virtual absl::StatusOr<std::string> WriteFile(std::string_view content) = 0;

  /// \brief Flush and finalize any outstanding writes.
  virtual absl::Status Close() = 0;
};

/// \brief Pimpl wrapper around IndexWriterInterface.
class IndexWriter final {
 public:
  /// \brief Constructs an IndexWriter from the provided implementation.
  explicit IndexWriter(std::unique_ptr<IndexWriterInterface> impl)
      : impl_(std::move(impl)) {}

  // IndexWriter is move-only.
  IndexWriter(IndexWriter&&) = default;
  IndexWriter& operator=(IndexWriter&&) = default;

  /// \brief Write the `IndexedCompilation` to the index.
  absl::StatusOr<std::string> WriteUnit(
      const kythe::proto::IndexedCompilation& unit) {
    return impl_->WriteUnit(unit);
  }

  /// \brief Write the file data to the index.
  absl::StatusOr<std::string> WriteFile(std::string_view content) {
    return impl_->WriteFile(content);
  }

  /// \brief Flush and finalize any outstanding writes.
  absl::Status Close() { return impl_->Close(); }

 private:
  std::unique_ptr<IndexWriterInterface> impl_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEX_WRITER_H_
