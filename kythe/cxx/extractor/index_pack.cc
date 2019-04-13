/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#include "index_pack.h"

#include <openssl/sha.h>
#include <uuid/uuid.h>

#include <utility>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "kythe/cxx/common/json_proto.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {
const char IndexPackFilesystem::kDataDirectoryName[] = "files";
const char IndexPackFilesystem::kCompilationUnitDirectoryName[] = "units";
const char IndexPackFilesystem::kFileDataSuffix[] = ".data";
const char IndexPackFilesystem::kCompilationUnitSuffix[] = ".unit";
const char IndexPackFilesystem::kTempFileSuffix[] = ".new";

std::unique_ptr<IndexPackPosixFilesystem> IndexPackPosixFilesystem::Open(
    const std::string& root_path, IndexPackFilesystem::OpenMode open_mode,
    std::string* error_text) {
  llvm::SmallString<256> abs_root(root_path);
  if (auto err = llvm::sys::fs::make_absolute(abs_root)) {
    *error_text = err.message();
    return nullptr;
  }
  auto filesystem = absl::make_unique<IndexPackPosixFilesystem>(
      abs_root.str(), open_mode, Token{});
  llvm::SmallString<256> unit_path = abs_root;
  llvm::sys::path::append(unit_path,
                          llvm::StringRef(kCompilationUnitDirectoryName));
  llvm::SmallString<256> data_path = abs_root;
  llvm::sys::path::append(data_path, llvm::StringRef(kDataDirectoryName));

  if (open_mode == OpenMode::kReadWrite) {
    if (auto err = llvm::sys::fs::create_directories(llvm::Twine(unit_path))) {
      *error_text = err.message();
      return nullptr;
    }
    if (auto err = llvm::sys::fs::create_directories(llvm::Twine(data_path))) {
      *error_text = err.message();
      return nullptr;
    }
  } else {
    bool is_dir;
    if (auto err =
            llvm::sys::fs::is_directory(llvm::Twine(unit_path), is_dir)) {
      *error_text = err.message();
      return nullptr;
    }
    if (!is_dir) {
      *error_text = std::string(unit_path.str()) + " is not a directory.";
      return nullptr;
    }
    if (auto err =
            llvm::sys::fs::is_directory(llvm::Twine(data_path), is_dir)) {
      *error_text = err.message();
      return nullptr;
    }
    if (!is_dir) {
      *error_text = std::string(data_path.str()) + " is not a directory.";
      return nullptr;
    }
  }
  filesystem->data_directory_ = data_path.str();
  filesystem->unit_directory_ = unit_path.str();
  return filesystem;
}

std::string IndexPackPosixFilesystem::GenerateFilenameFor(
    DataKind data_kind, const std::string& hash, std::string* error_text) {
  if (hash.size() != 64) {
    *error_text = "Invalid name: bad SHA256 digest length.";
    return "";
  }
  // This also takes care of bad hashes with path separators or extensions.
  for (char c : hash) {
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
      *error_text = "Invalid name: name is not a valid lowercase SHA256 digest";
      return "";
    }
  }
  llvm::SmallString<256> temp_path(directory_for(data_kind));
  llvm::sys::path::append(temp_path, hash + extension_for(data_kind));
  return temp_path.str();
}

/// \brief Represents a single UUID, generated during construction.
class Uuid {
 public:
  Uuid() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    // "The uuid_unparse function converts the supplied UUID uu from the binary
    // representation into a 36-byte string (plus tailing '\0')"
    char uuid_buffer[37];
    uuid_unparse_lower(uuid, uuid_buffer);
    payload_ = uuid_buffer;
  }

  /// \brief Returns a UUID (if ok()) or an error string (if !ok()).
  const std::string& payload() { return payload_; }

  /// \brief Checks whether the uuid generated correctly.
  bool ok() { return ok_; }

 private:
  /// Error text (if !ok_) or a UUID string (if ok_).
  std::string payload_;
  /// Determines whether UUID generation was successful.
  bool ok_ = true;
};

/// \brief Opens a new file with a unique name in some directory.
/// \param abs_root_directory The absolute path to the directory.
/// \param fd_out Will be set to the fd of the open file.
/// \param path_out Will be set to the path of the open file.
/// \param error_text Set to an error description on failure.
/// \return true on success, false on failure (fd_out, path_out are unset).
static bool OpenUniqueTempFileIn(const std::string& abs_root_directory,
                                 int* fd_out, std::string* path_out,
                                 std::string* error_text) {
  for (;;) {
    Uuid new_uuid;
    if (!new_uuid.ok()) {
      *error_text = new_uuid.payload();
      return false;
    }
    llvm::SmallString<256> path(abs_root_directory);
    llvm::sys::path::append(
        path, new_uuid.payload() + IndexPackFilesystem::kTempFileSuffix);
    if (auto err = llvm::sys::fs::openFileForWrite(
            llvm::Twine(path), *fd_out, llvm::sys::fs::CD_CreateNew,
            llvm::sys::fs::OF_None,
            llvm::sys::fs::all_read | llvm::sys::fs::all_write)) {
      if (err != std::errc::file_exists) {
        *error_text = err.message();
        return false;
      }
    } else {
      *path_out = path.str();
      return true;
    }
  }
}

bool IndexPackPosixFilesystem::ReadFileContent(DataKind data_kind,
                                               const std::string& file_name,
                                               ReadCallback callback,
                                               std::string* error_text) {
  std::string file = GenerateFilenameFor(data_kind, file_name, error_text);
  if (file.empty()) {
    return false;
  }
  int in_fd;
  if (auto err = llvm::sys::fs::openFileForRead(llvm::Twine(file), in_fd)) {
    *error_text = err.message() + " (" + file + ")";
    return false;
  }
  google::protobuf::io::FileInputStream file_stream(in_fd);
  google::protobuf::io::GzipInputStream stream(
      &file_stream, google::protobuf::io::GzipInputStream::Format::GZIP);
  bool user_result = callback(&stream, error_text);
  if (const char* err = stream.ZlibErrorMessage()) {
    *error_text = err;
    file_stream.Close();
    return false;
  }
  if (!file_stream.Close()) {
    *error_text = "Could not close file input stream.";
    return false;
  }
  return user_result;
}

bool IndexPackPosixFilesystem::ScanFiles(DataKind data_kind,
                                         ScanCallback callback,
                                         std::string* error_text) {
  std::error_code err;
  llvm::sys::fs::directory_iterator current(
      llvm::Twine(directory_for(data_kind)), err),
      end;
  if (err) {
    *error_text = err.message();
    return false;
  }
  for (; current != end; current = current.increment(err)) {
    if (err) {
      *error_text = err.message();
      return false;
    }
    const std::string& path = current->path();
    // Is the path well-formed for this kind?
    llvm::StringRef path_ref(path);
    if (llvm::sys::path::parent_path(path_ref) != directory_for(data_kind)) {
      *error_text = "Invalid file in index pack: " + path;
      return false;
    }
    auto extension = llvm::sys::path::extension(path_ref);
    if (extension != extension_for(data_kind)) {
      // Ignore files we don't understand.
      continue;
    }
    if (!callback(llvm::sys::path::stem(path).str())) {
      return true;
    }
  }
  return true;
}

bool IndexPackPosixFilesystem::AddFileContent(DataKind data_kind,
                                              WriteCallback callback,
                                              std::string* error_text) {
  if (open_mode_ != OpenMode::kReadWrite) {
    *error_text = "Index pack not opened for writing.";
    return false;
  }
  std::string temp_path;
  int temp_fd;
  if (!OpenUniqueTempFileIn(directory_for(data_kind), &temp_fd, &temp_path,
                            error_text)) {
    return false;
  }
  google::protobuf::io::FileOutputStream file_stream(temp_fd);
  google::protobuf::io::GzipOutputStream::Options options;
  options.format = google::protobuf::io::GzipOutputStream::GZIP;
  google::protobuf::io::GzipOutputStream stream(&file_stream, options);
  std::string file_hash;
  auto callback_result = callback(&stream, &file_hash, error_text);
  if (!callback_result) {
    return callback_result;
  }
  if (!stream.Close()) {
    *error_text = "Couldn't close gzip output stream.";
    return false;
  }
  if (!file_stream.Close()) {
    *error_text = "Couldn't close file output stream.";
    return false;
  }
  std::string file = GenerateFilenameFor(data_kind, file_hash, error_text);
  if (file.empty()) {
    return false;
  }
  auto err = llvm::sys::fs::rename(llvm::Twine(temp_path), llvm::Twine(file));
  if (err) {
    llvm::sys::fs::remove(llvm::Twine(temp_path));
    *error_text = err.message();
    return false;
  }
  return true;
}

// We need "the lowercase ascii hex SHA-256 digest of the file contents."
static constexpr char kHexDigits[] = "0123456789abcdef";

/// TODO(zarko): Move this into its own common header.
/// \brief Returns the lowercase-string-hex-encoded sha256 digest of the first
/// `length` bytes of `bytes`.
static std::string Sha256(const void* bytes, size_t length) {
  unsigned char sha_buf[SHA256_DIGEST_LENGTH];
  ::SHA256(reinterpret_cast<const unsigned char*>(bytes), length, sha_buf);
  std::string sha_text(SHA256_DIGEST_LENGTH * 2, '\0');
  for (unsigned i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    sha_text[i * 2] = kHexDigits[(sha_buf[i] >> 4) & 0xF];
    sha_text[i * 2 + 1] = kHexDigits[sha_buf[i] & 0xF];
  }
  return sha_text;
}

bool IndexPack::AddCompilationUnit(const kythe::proto::CompilationUnit& unit,
                                   std::string* error_text) {
  return WriteMessage(IndexPackFilesystem::DataKind::kCompilationUnit, unit,
                      error_text);
}

bool IndexPack::ReadFileData(const std::string& hash, std::string* out) {
  return filesystem_->ReadFileContent(
      IndexPackFilesystem::DataKind::kFileData, hash,
      [out](google::protobuf::io::ZeroCopyInputStream* stream,
            std::string* error_text) {
        out->clear();
        const void* data;
        int size;
        // The callback's caller will check for errors.
        while (stream->Next(&data, &size)) {
          if (size <= 0) {
            continue;
          }
          out->append(static_cast<const char*>(data), size);
        }
        return true;
      },
      out);
}

bool IndexPack::ReadCompilationUnit(const std::string& hash,
                                    kythe::proto::CompilationUnit* unit,
                                    std::string* error_text) {
  // TODO(zarko): Wrap the input stream and deserialize from it without
  // the buffer in between.
  std::string buffer;
  if (!filesystem_->ReadFileContent(
          IndexPackFilesystem::DataKind::kCompilationUnit, hash,
          [&buffer](google::protobuf::io::ZeroCopyInputStream* stream,
                    std::string* error_text) {
            buffer.clear();
            const void* data;
            int size;
            // The callback's caller will check for errors.
            while (stream->Next(&data, &size)) {
              if (size <= 0) {
                continue;
              }
              buffer.append(static_cast<const char*>(data), size);
            }
            return true;
          },
          error_text)) {
    return false;
  }
  std::string format_string;
  if (!MergeJsonWithMessage(buffer, &format_string, unit)) {
    *error_text = "Invalid compilation unit: " + hash;
    return false;
  }
  if (format_string != "kythe") {
    *error_text = "Unsupported format for compilation unit " + hash + ": " +
                  format_string;
    return false;
  }
  return true;
}

bool IndexPack::ScanData(IndexPackFilesystem::DataKind kind,
                         IndexPackFilesystem::ScanCallback callback,
                         std::string* error_text) {
  return filesystem_->ScanFiles(kind, std::move(callback), error_text);
}

bool IndexPack::AddFileData(const kythe::proto::FileData& content,
                            std::string* error_text) {
  std::string digest;
  if (content.has_info()) {
    digest = content.info().digest();
  }
  return WriteData(IndexPackFilesystem::DataKind::kFileData,
                   content.content().data(), content.content().size(),
                   error_text, digest.empty() ? nullptr : &digest);
}

bool IndexPack::WriteMessage(IndexPackFilesystem::DataKind kind,
                             const google::protobuf::Message& message,
                             std::string* error_text) {
  // TODO(zarko): Wrap the output stream and serialize to it without the
  // buffer in between. (This is why the filename is an out-parameter of
  // the callback to AddFileContent--we might have to calculate the hash
  // on the fly, so we won't know it until we're done serializing.)
  std::string message_content;
  if (!WriteMessageAsJsonToString(message, "kythe", &message_content)) {
    *error_text = "Couldn't serialize message.";
    return false;
  }
  return WriteData(kind, message_content.data(), message_content.size(),
                   error_text, nullptr);
}

bool IndexPack::WriteData(IndexPackFilesystem::DataKind kind, const char* data,
                          size_t size, std::string* error_text,
                          std::string* sha_in) {
  std::string sha = sha_in ? *sha_in : Sha256(data, size);
  return filesystem_->AddFileContent(
      kind,
      [data, size, &sha](google::protobuf::io::ZeroCopyOutputStream* stream,
                         std::string* file_name, std::string* error_text) {
        size_t bytes_left = size;
        while (bytes_left) {
          void* buffer;
          int buffer_size;
          if (!stream->Next(&buffer, &buffer_size) || buffer_size < 0) {
            *error_text = "Can't allocate buffer.";
            return false;
          }
          int chunk_size =
              static_cast<int>(std::min<size_t>(bytes_left, buffer_size));
          size_t start_offset = size - bytes_left;
          ::memcpy(buffer, data + start_offset, chunk_size);
          if (buffer_size > chunk_size) {
            stream->BackUp(buffer_size - chunk_size);
          }
          bytes_left -= chunk_size;
        }
        *file_name = sha;
        return true;
      },
      error_text);
}
}  // namespace kythe
