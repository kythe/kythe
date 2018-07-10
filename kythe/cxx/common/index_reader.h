#ifndef KYTHE_CXX_COMMON_INDEX_READER_H_
#define KYTHE_CXX_COMMON_INDEX_READER_H_

#include <functional>
#include <memory>

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

  /// \brief Reads and returns the request file data.
  ///  Returns kNotFound if the digest isn't present.
  virtual StatusOr<std::unique_ptr<char[]>> ReadFile(
      absl::string_view digest) = 0;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEX_READER_H_
