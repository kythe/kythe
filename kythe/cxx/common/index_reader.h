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

#ifndef KYTHE_CXX_COMMON_INDEX_READER_H_
#define KYTHE_CXX_COMMON_INDEX_READER_H_

#include <functional>
#include <string>
#include <string_view>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief Simple interface for reading IndexedCompilations and files
/// from an underlying data store.
class IndexReaderInterface {
 public:
  /// \brief Callback invoked for each available unit digest.
  using ScanCallback = std::function<bool(std::string_view)>;

  IndexReaderInterface() = default;
  // IndexReaderInterface is neither copyable nor movable.
  IndexReaderInterface(const IndexReaderInterface&) = delete;
  IndexReaderInterface& operator=(const IndexReaderInterface&) = delete;
  virtual ~IndexReaderInterface() = default;

  /// \brief Invokes `scan` for each IndexedCompilation unit digest or until it
  /// returns false.
  virtual absl::Status Scan(const ScanCallback& scan) = 0;

  /// \brief Reads and returns requested IndexCompilation.
  ///  Returns kNotFound if the digest isn't present.
  virtual absl::StatusOr<kythe::proto::IndexedCompilation> ReadUnit(
      std::string_view digest) = 0;

  /// \brief Reads and returns the requested file data.
  ///  Returns kNotFound if the digest isn't present.
  virtual absl::StatusOr<std::string> ReadFile(std::string_view digest) = 0;
};

/// \brief Pimpl wrapper around IndexReaderInterface.
class IndexReader {
 public:
  using ScanCallback = IndexReaderInterface::ScanCallback;

  /// \brief Constructs an IndexReader from the provided implementation.
  explicit IndexReader(std::unique_ptr<IndexReaderInterface> impl)
      : impl_(std::move(impl)) {}

  // IndexReader is move-only.
  IndexReader(IndexReader&&) = default;
  IndexReader& operator=(IndexReader&&) = default;

  /// \brief Invokes `scan` for each IndexedCompilation unit digest or until it
  /// returns false.
  absl::Status Scan(const ScanCallback& scan) { return impl_->Scan(scan); }

  /// \brief Reads and returns requested IndexCompilation.
  ///  Returns kNotFound if the digest isn't present.
  absl::StatusOr<kythe::proto::IndexedCompilation> ReadUnit(
      std::string_view digest) {
    return impl_->ReadUnit(digest);
  }

  /// \brief Reads and returns the requested file data.
  ///  Returns kNotFound if the digest isn't present.
  absl::StatusOr<std::string> ReadFile(std::string_view digest) {
    return impl_->ReadFile(digest);
  }

 private:
  std::unique_ptr<IndexReaderInterface> impl_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEX_READER_H_
