#ifndef KYTHE_CXX_COMMON_KZIP_READER_H
#define KYTHE_CXX_COMMON_KZIP_READER_H

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

#endif  // KYTHE_CXX_COMMON_KZIP_READER_H
