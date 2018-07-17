#ifndef KYTHE_CXX_COMMON_INDEX_WRITER_H_
#define KYTHE_CXX_COMMON_INDEX_WRITER_H_

#include "kythe/cxx/common/status.h"
#include "kythe/proto/analysis.pb.h"

#include "absl/strings/string_view.h"

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

  /// \brief Write the `IndexedCompilation` to the index.
  virtual Status WriteUnit(const kythe::proto::IndexedCompilation& unit) = 0;

  /// \brief Write the file data to the index.
  virtual Status WriteFile(absl::string_view content) = 0;

  /// \brief Flush and finalize any outstanding writes.
  virtual Status Close() = 0;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEX_WRITER_H_
