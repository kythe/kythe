#ifndef KYTHE_CXX_COMMON_INDEX_READER_H_
#define KYTHE_CXX_COMMON_INDEX_READER_H_

#include <functional>
#include <string>

#include "absl/strings/string_view.h"
#include "kythe/cxx/common/status_or.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief Simple interface for reading IndexedCompilations and files
/// from an underlying data store.
class IndexReaderInterface {
 public:
  /// \brief Callback invoked for each available unit digest.
  using ScanCallback = std::function<bool(absl::string_view)>;

  IndexReaderInterface() = default;
  // IndexReaderInterface is neither copyable nor movable.
  IndexReaderInterface(const IndexReaderInterface&) = delete;
  IndexReaderInterface& operator=(const IndexReaderInterface&) = delete;
  virtual ~IndexReaderInterface() = default;

  /// \brief Invokes `scan` for each IndexedCompilation unit digest or until it
  /// returns false.
  virtual Status Scan(const ScanCallback& scan) = 0;

  /// \brief Reads and returns requested IndexCompilation.
  ///  Returns kNotFound if the digest isn't present.
  virtual StatusOr<kythe::proto::IndexedCompilation> ReadUnit(
      absl::string_view digest) = 0;

  /// \brief Reads and returns the requested file data.
  ///  Returns kNotFound if the digest isn't present.
  virtual StatusOr<std::string> ReadFile(absl::string_view digest) = 0;
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
  Status Scan(const ScanCallback& scan) { return impl_->Scan(scan); }

  /// \brief Reads and returns requested IndexCompilation.
  ///  Returns kNotFound if the digest isn't present.
  StatusOr<kythe::proto::IndexedCompilation> ReadUnit(
      absl::string_view digest) {
    return impl_->ReadUnit(digest);
  }

  /// \brief Reads and returns the requested file data.
  ///  Returns kNotFound if the digest isn't present.
  StatusOr<std::string> ReadFile(absl::string_view digest) {
    return impl_->ReadFile(digest);
  }

 private:
  std::unique_ptr<IndexReaderInterface> impl_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEX_READER_H_
