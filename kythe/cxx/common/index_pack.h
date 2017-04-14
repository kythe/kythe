/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_INDEX_PACK_H_
#define KYTHE_CXX_COMMON_INDEX_PACK_H_

#include <memory>
#include <string>

#include "google/protobuf/io/zero_copy_stream.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief The `IndexPack`'s interface to the filesystem.
class IndexPackFilesystem {
 public:
  /// \brief Modes in which an index pack may be opened.
  enum class OpenMode {
    kReadOnly,  ///< This index pack is read-only.
    kReadWrite  ///< This index pack may write and read.
  };

  /// \brief Kinds of data that might be written.
  enum class DataKind {
    kFileData,        ///< File content.
    kCompilationUnit  ///< Compilation unit metadata.
  };

  /// \brief A callback to get file data and a filename.
  /// \param stream The stream to write to.
  /// \param file_name Non-null; the name of the file we're writing (without
  /// extension).
  /// \param error_text Non-null; used for error descriptions.
  /// \return false on failure and true on success.
  using WriteCallback =
      std::function<bool(google::protobuf::io::ZeroCopyOutputStream *stream,
                         std::string *file_name, std::string *error_text)>;

  /// \brief Attempt to add file content to the underlying index pack.
  /// \param data_kind The kind of data this file represents.
  /// \param callback Callback to actually write the data.
  /// \param error_text Non-null; used to return error details.
  /// \return false on failure and true on success.
  virtual bool AddFileContent(DataKind data_kind, WriteCallback callback,
                              std::string *error_text) {
    return false;
  }

  /// \brief A callback to provide file data.
  /// \param stream The stream to read from.
  /// \param error_text Non-null; used for error descriptions.
  /// \return false on failure or true for success.
  using ReadCallback =
      std::function<bool(google::protobuf::io::ZeroCopyInputStream *stream,
                         std::string *error_text)>;

  /// \brief Attempt to read file content from the underlying index pack.
  /// \param data_kind The kind of data this file represents.
  /// \param callback Callback to actually read the data.
  /// \param error_text Non-null; used for error descriptions.
  /// \return false on failure or true for success.
  virtual bool ReadFileContent(DataKind data_kind, const std::string &file_name,
                               ReadCallback callback, std::string *error_text) {
    return false;
  }

  /// \brief A callback to provide a filename during a scan.
  /// \param file_name The name of the next file (without extension).
  /// \return true to continue scanning; false to stop.
  using ScanCallback = std::function<bool(const std::string &file_name)>;

  /// \brief Attempt to scan all files of a particular kind.
  /// \param data_kind The kind of data to scan.
  /// \param callback Callback issued for each file found.
  /// \param error_text Non-null; used for error descriptions.
  /// \return false on failure or true for success (including aborted scans).
  virtual bool ScanFiles(DataKind data_kind, ScanCallback callback,
                         std::string *error_text) {
    return false;
  }

  /// \brief The directory name to use for file data.
  static const char kDataDirectoryName[];

  /// \brief The directory name to use for compilation unit data.
  static const char kCompilationUnitDirectoryName[];

  /// \brief The suffix to use for file data.
  static const char kFileDataSuffix[];

  /// \brief The suffix to use for compliation unit data.
  static const char kCompilationUnitSuffix[];

  /// \brief The suffix to use for temporary file data.
  static const char kTempFileSuffix[];

  /// \brief Returns the mode in which this index pack has been opened.
  virtual OpenMode open_mode() const = 0;

  virtual ~IndexPackFilesystem() {}
};

/// \brief A read/write `IndexPackFilesystem` that uses atomic renames.
class IndexPackPosixFilesystem : public IndexPackFilesystem {
 public:
  /// \brief Mounts a subdirectory as an index pack.
  /// \param root_path The root path to use.
  /// \param open_mode Whether to mount the root path read-only.
  /// \param error_text A pointer to a string to use to fill with an error
  /// description. Must not be null.
  ///
  /// If `open_mode` is `kReadWrite`, `Open` will attempt to create the
  /// index pack directory structure if it does not already exist.
  ///
  /// \return A new `IndexPackPosixFilesystem`, or `null` on error.
  static std::unique_ptr<IndexPackPosixFilesystem> Open(
      const std::string &root_path, IndexPackFilesystem::OpenMode open_mode,
      std::string *error_text);

  IndexPackFilesystem::OpenMode open_mode() const override {
    return open_mode_;
  }

  bool AddFileContent(DataKind data_kind, WriteCallback callback,
                      std::string *error_text) override;

  bool ReadFileContent(DataKind data_kind, const std::string &file_name,
                       ReadCallback callback, std::string *error_text) override;

  bool ScanFiles(DataKind data_kind, ScanCallback callback,
                 std::string *error_text) override;

 private:
  /// \brief Build an IndexPackPosixFilesystem without verifying that it's OK.
  /// \param root_directory The mount point as an absolute path.
  /// \param open_mode Mount as read-only or read/write?
  IndexPackPosixFilesystem(std::string root_directory,
                           IndexPackFilesystem::OpenMode open_mode)
      : root_directory_(root_directory), open_mode_(open_mode) {}

  /// \brief Returns the absolute path to the directory for storing data.
  const std::string &directory_for(DataKind data_kind) const {
    return data_kind == DataKind::kFileData ? data_directory_ : unit_directory_;
  }

  /// \brief Returns the extension used for storing data.
  static const char *extension_for(DataKind data_kind) {
    return data_kind == DataKind::kFileData ? kFileDataSuffix
                                            : kCompilationUnitSuffix;
  }

  /// \brief Build a path for a resource.
  /// \return The empty string if the hash is invalid; otherwise, the path.
  std::string GenerateFilenameFor(DataKind data_kind, const std::string &hash,
                                  std::string *error_text);

  /// Where the index pack is mounted in the external filesystem (absolute).
  std::string root_directory_;
  /// This filesystem's read/write status.
  IndexPackFilesystem::OpenMode open_mode_;
  /// The path to the data directory (absolute).
  std::string data_directory_;
  /// The path to the unit directory (absolute).
  std::string unit_directory_;
};

/// \brief A collection of compilation units and associated file data.
class IndexPack {
 public:
  /// \brief Constructs an IndexPack using the provided filesystem.
  /// \param filesystem The filesystem. May be read-only or read/write.
  explicit IndexPack(std::unique_ptr<IndexPackFilesystem> filesystem)
      : filesystem_(std::move(filesystem)) {}

  /// \brief Adds a CompilationUnit to the index pack.
  /// \param unit The `CompilationUnit` to add.
  /// \param error_text Set if the return value is false.
  /// \return false on failure and true on success.
  bool AddCompilationUnit(const kythe::proto::CompilationUnit &unit,
                          std::string *error_text);

  /// \brief Adds file data to the index pack.
  /// \param content The `FileData` to add.
  /// \param error_text Set if the return value is false.
  /// \return false on failure and true on success.
  ///
  /// If the digest of `content` is set, it will not be recomputed.
  /// Fields besides `content` and `digest` on `content` are ignored.
  bool AddFileData(const kythe::proto::FileData &content,
                   std::string *error_text);

  /// \brief Reads file data from the index pack.
  /// \param hash The hash of the file to read.
  /// \param out Non-null. On success, contains the file data. On failure,
  /// contains error text.
  /// \return true on success; false on failure.
  bool ReadFileData(const std::string &hash, std::string *out);

  /// \brief Reads a `CompilationUnit` from the index pack.
  /// \param hash The hash of the unit to read.
  /// \param out Non-null. On success, becomes the unit read from the pack.
  /// \param error_text Non-null. On failure, becomes an error description.
  /// \return true on success; false on failure.
  bool ReadCompilationUnit(const std::string &hash,
                           kythe::proto::CompilationUnit *unit,
                           std::string *error_text);

  /// \brief Scans over the hashes of various data in the index pack.
  /// \param kind The kind of data to scan.
  /// \param callback The callback to call with each hash. Return true to
  /// continue scanning and false to stop.
  /// \param error_text Set to text describing errors should they occur.
  /// \return true if no errors (even if the callback stopped early).
  bool ScanData(IndexPackFilesystem::DataKind kind,
                std::function<bool(const std::string &hash)> callback,
                std::string *error_text);

 private:
  /// \brief Write data of kind `kind` with payload `message`.
  /// \return false on failure and true on success.
  bool WriteMessage(IndexPackFilesystem::DataKind kind,
                    const google::protobuf::Message &message,
                    std::string *error_text);

  /// \brief Write data of kind `kind` with some raw payload.
  /// \param sha If null, recalculates the SHA256 digest of the data.
  /// \return false on failure and true on success.
  bool WriteData(IndexPackFilesystem::DataKind kind, const char *bytes,
                 size_t size, std::string *error_text,
                 std::string *sha = nullptr);

  /// The view of the filesystem for this IndexPack.
  std::unique_ptr<IndexPackFilesystem> filesystem_;
};
}

#endif  // KYTHE_CXX_COMMON_INDEX_PACK_H_
