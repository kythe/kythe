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

#include "kythe/cxx/common/kzip_reader.h"

#include <errno.h>
#include <iostream>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/types/optional.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/json_proto.h"

namespace kythe {
namespace {
struct ZipFileClose {
  void operator()(zip_file_t* file) {
    if (file != nullptr) {
      CHECK(zip_fclose(file) == 0);
    }
  }
};
using ZipFile = std::unique_ptr<zip_file_t, ZipFileClose>;

class ZipFileInputStream : public google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit ZipFileInputStream(zip_file_t* file) : input_(file) {}

  bool Next(const void** data, int* size) override {
    return impl_.Next(data, size);
  }

  void BackUp(int count) override { impl_.BackUp(count); }
  bool Skip(int count) override { return impl_.Skip(count); }
  google::protobuf::int64 ByteCount() const override {
    return impl_.ByteCount();
  }

 private:
  class CopyingZipInputStream
      : public google::protobuf::io::CopyingInputStream {
   public:
    explicit CopyingZipInputStream(zip_file_t* file) : file_(file) {}

    int Read(void* buffer, int size) override {
      return zip_fread(file_, buffer, size);
    }

    int Skip(int count) override {
      zip_int64_t start = zip_ftell(file_);
      if (start < 0) {
        return 0;
      }
      if (zip_fseek(file_, count, SEEK_CUR) < 0) {
        return 0;
      }
      zip_int64_t end = zip_ftell(file_);
      if (end < 0) {
        return 0;
      }
      return end - start;
    }

   private:
    zip_file_t* file_;
  };

  CopyingZipInputStream input_;
  google::protobuf::io::CopyingInputStreamAdaptor impl_{&input_};
};

absl::optional<absl::string_view> UnitDigest(absl::string_view path) {
  path.remove_prefix(std::min(path.find('/'), path.size()));
  if (!absl::ConsumePrefix(&path, "/units/")) {
    return absl::nullopt;
  }
  return path;
}

StatusOr<absl::string_view> Validate(zip_t* archive) {
  if (!zip_get_num_entries(archive, 0)) {
    return InvalidArgumentError("Empty kzip archive");
  }

  // Pull the root directory from an arbitrary entry.
  absl::string_view root = zip_get_name(archive, 0, 0);
  auto slashpos = root.find('/');
  if (slashpos == 0 || slashpos == absl::string_view::npos) {
    return InvalidArgumentError(
        absl::StrCat("Malformed kzip: invalid root: ", root));
  }
  root.remove_suffix(root.size() - slashpos);
  LOG(INFO) << "Using archive root: " << root;
  for (int i = 0; i < zip_get_num_entries(archive, 0); ++i) {
    absl::string_view name = zip_get_name(archive, i, 0);
    if (!absl::StartsWith(name, root)) {
      return InvalidArgumentError(
          absl::StrCat("Malformed kzip: invalid entry: ", name));
    }
  }
  return root;
}

StatusCode ZlibStatusCode(int zip_error) {
  switch (zip_error) {
    case ZIP_ER_OK:  // No error
      return StatusCode::kOk;
    case ZIP_ER_MULTIDISK:    // Multi-disk zip archives not supported
    case ZIP_ER_COMPNOTSUPP:  // Compression method not supported
    case ZIP_ER_ENCRNOTSUPP:  // Encryption method not supported
      return StatusCode::kUnimplemented;
    case ZIP_ER_INVAL:            // Invalid argument
    case ZIP_ER_NOZIP:            // Not a zip archive
    case ZIP_ER_INCONS:           // Zip archive inconsistent
    case ZIP_ER_COMPRESSED_DATA:  // Compressed data invalid
    case ZIP_ER_CRC:              // CRC error
      return StatusCode::kInvalidArgument;
    case ZIP_ER_RENAME:    // Renaming temporary file failed
    case ZIP_ER_CLOSE:     // Closing zip archive failed
    case ZIP_ER_SEEK:      // Seek error
    case ZIP_ER_READ:      // Read error
    case ZIP_ER_WRITE:     // Write error
    case ZIP_ER_ZLIB:      // Zlib error
    case ZIP_ER_INTERNAL:  // Internal error
    case ZIP_ER_REMOVE:    // Can't remove file
    case ZIP_ER_TELL:      // Tell error
      return StatusCode::kInternal;
    case ZIP_ER_ZIPCLOSED:  // Containing zip archive was closed
    case ZIP_ER_INUSE:      // Resource still in use
      return StatusCode::kFailedPrecondition;
    case ZIP_ER_NOENT:    // No such file
    case ZIP_ER_DELETED:  // Entry has been deleted
      return StatusCode::kNotFound;
    case ZIP_ER_EXISTS:  // File already exists
      return StatusCode::kAlreadyExists;
    case ZIP_ER_MEMORY:  // Malloc failure
      return StatusCode::kResourceExhausted;
    case ZIP_ER_CHANGED:  // Entry has been changed
      return StatusCode::kAborted;
    case ZIP_ER_EOF:  // Premature end of file
      return StatusCode::kOutOfRange;
    case ZIP_ER_OPEN:     // Can't open file
    case ZIP_ER_TMPOPEN:  // Failure to create temporary file
    default:
      return StatusCode::kUnknown;
  }
}

StatusCode SystemStatusCode(int sys_error) {
  switch (sys_error) {
    case 0:
      return StatusCode::kOk;
    case EINVAL:        // Invalid argument
    case ENAMETOOLONG:  // Filename too long
    case E2BIG:         // Argument list too long
    case EDESTADDRREQ:  // Destination address required
    case EDOM:          // Mathematics argument out of domain of function
    case EFAULT:        // Bad address
    case EILSEQ:        // Illegal byte sequence
    case ENOPROTOOPT:   // Protocol not available
    case ENOSTR:        // Not a STREAM
    case ENOTSOCK:      // Not a socket
    case ENOTTY:        // Inappropriate I/O control operation
    case EPROTOTYPE:    // Protocol wrong type for socket
    case ESPIPE:        // Invalid seek
      return StatusCode::kInvalidArgument;
    case ETIMEDOUT:  // Connection timed out
    case ETIME:      // Timer expired
      return StatusCode::kDeadlineExceeded;
    case ENODEV:     // No such device
    case ENOENT:     // No such file or directory
    case ENOMEDIUM:  // No medium found
    case ENXIO:      // No such device or address
    case ESRCH:      // No such process
      return StatusCode::kNotFound;
    case EEXIST:         // File exists
    case EADDRNOTAVAIL:  // Address not available
    case EALREADY:       // Connection already in progress
    case ENOTUNIQ:       // Name not unique on network
      return StatusCode::kAlreadyExists;
    case EPERM:   // Operation not permitted
    case EACCES:  // Permission denied
    case ENOKEY:  // Required key not available
    case EROFS:   // Read only file system
      return StatusCode::kPermissionDenied;
    case ENOTEMPTY:   // Directory not empty
    case EISDIR:      // Is a directory
    case ENOTDIR:     // Not a directory
    case EADDRINUSE:  // Address already in use
    case EBADF:       // Invalid file descriptor
    case EBADFD:      // File descriptor in bad state
    case EBUSY:       // Device or resource busy
    case ECHILD:      // No child processes
    case EISCONN:     // Socket is connected
    case EISNAM:      // Is a named type file
    case ENOTBLK:     // Block device required
    case ENOTCONN:    // The socket is not connected
    case EPIPE:       // Broken pipe
    case ESHUTDOWN:   // Cannot send after transport endpoint shutdown
    case ETXTBSY:     // Text file busy
    case EUNATCH:     // Protocol driver not attached
      return StatusCode::kFailedPrecondition;
    case ENOSPC:   // No space left on device
    case EDQUOT:   // Disk quota exceeded
    case EMFILE:   // Too many open files
    case EMLINK:   // Too many links
    case ENFILE:   // Too many open files in system
    case ENOBUFS:  // No buffer space available
    case ENODATA:  // No message is available on the STREAM read queue
    case ENOMEM:   // Not enough space
    case ENOSR:    // No STREAM resources
    case EUSERS:   // Too many users
      return StatusCode::kResourceExhausted;
    case ECHRNG:     // Channel number out of range
    case EFBIG:      // File too large
    case EOVERFLOW:  // Value too large to be stored in data type
    case ERANGE:     // Result too large
      return StatusCode::kOutOfRange;
    case ENOPKG:           // Package not installed
    case ENOSYS:           // Function not implemented
    case ENOTSUP:          // Operation not supported
    case EAFNOSUPPORT:     // Address family not supported
    case EPFNOSUPPORT:     // Protocol family not supported
    case EPROTONOSUPPORT:  // Protocol not supported
    case ESOCKTNOSUPPORT:  // Socket type not supported
    case EXDEV:            // Improper link
      return StatusCode::kUnimplemented;
    case EAGAIN:        // Resource temporarily unavailable
    case ECOMM:         // Communication error on send
    case ECONNREFUSED:  // Connection refused
    case ECONNABORTED:  // Connection aborted
    case ECONNRESET:    // Connection reset
    case EINTR:         // Interrupted function call
    case EHOSTDOWN:     // Host is down
    case EHOSTUNREACH:  // Host is unreachable
    case ENETDOWN:      // Network is down
    case ENETRESET:     // Connection aborted by network
    case ENETUNREACH:   // Network unreachable
    case ENOLCK:        // No locks available
    case ENOLINK:       // Link has been severed
    case ENONET:        // Machine is not on the network
      return StatusCode::kUnavailable;
    case EDEADLK:  // Resource deadlock avoided
    case ESTALE:   // Stale file handle
      return StatusCode::kAborted;
    case ECANCELED:  // Operation cancelled
      return StatusCode::kCancelled;
    default:
      return StatusCode::kUnknown;
  }
}

StatusCode GetStatusCode(zip_error_t* error) {
  switch (zip_error_system_type(error)) {
    case ZIP_ET_SYS:
      return SystemStatusCode(zip_error_code_system(error));
    case ZIP_ET_ZLIB:
      return ZlibStatusCode(zip_error_code_system(error));
    case ZIP_ET_NONE:
    default:
      return ZlibStatusCode(zip_error_code_zip(error));
  }
}

Status ToStatus(zip_error_t* error) {
  StatusCode code = GetStatusCode(error);
  if (code == StatusCode::kOk) {
    return OkStatus();
  }
  return Status(code, zip_error_strerror(error));
}

absl::optional<zip_uint64_t> FileSize(zip_t* archive, zip_uint64_t index) {
  zip_stat_t sb;
  zip_stat_init(&sb);

  if (zip_stat_index(archive, index, ZIP_STAT_SIZE, &sb) < 0) {
    return absl::nullopt;
  }
  return sb.size;
}

StatusOr<std::string> ReadTextFile(zip_t* archive, const std::string& path) {
  zip_int64_t index = zip_name_locate(archive, path.c_str(), 0);
  if (index >= 0) {
    if (auto file = ZipFile(zip_fopen_index(archive, index, 0))) {
      if (auto size = FileSize(archive, index)) {
        std::string result(*size, '\0');
        if (zip_fread(file.get(), &result.front(), *size) == *size) {
          return result;
        } else {
          return ToStatus(zip_file_get_error(file.get()));
        }
      }
    }
  }
  Status status = ToStatus(zip_get_error(archive));
  if (!status.ok()) {
    return status;
  }
  return UnknownError(absl::StrCat("Unable to read: ", path));
}

}  // namespace

/* static */
StatusOr<IndexReader> KzipReader::Open(absl::string_view path) {
  // TODO(shahms): Support opening a zip_source_t wrapper class of some sort.
  int error;
  if (auto archive =
          ZipHandle(zip_open(std::string(path).c_str(), ZIP_RDONLY, &error))) {
    if (auto root = Validate(archive.get())) {
      return IndexReader(
          absl::WrapUnique(new KzipReader(std::move(archive), *root)));
    } else {
      return root.status();
    }
  }
  return Status(ZlibStatusCode(error), absl::StrCat("Unable to open: ", path));
}

KzipReader::KzipReader(ZipHandle archive, absl::string_view root)
    : archive_(std::move(archive)), root_(root) {}

StatusOr<proto::IndexedCompilation> KzipReader::ReadUnit(
    absl::string_view digest) {
  std::string path = absl::StrCat(root_, "/units/", digest);
  if (auto file = ZipFile(zip_fopen(archive(), path.c_str(), 0))) {
    proto::IndexedCompilation unit;
    ZipFileInputStream input(file.get());
    Status status = ParseFromJsonStream(&input, &unit);
    if (!status.ok()) {
      Status zip_status = ToStatus(zip_file_get_error(file.get()));
      if (!zip_status.ok()) {
        // Prefer the underlying zip error, if present.
        return zip_status;
      }
      return status;
    }
    return unit;
  }
  Status status = ToStatus(zip_get_error(archive()));
  if (!status.ok()) {
    return status;
  }
  return UnknownError(absl::StrCat("Unable to open unit ", digest));
}

StatusOr<std::string> KzipReader::ReadFile(absl::string_view digest) {
  return ReadTextFile(archive(), absl::StrCat(root_, "/files/", digest));
}

Status KzipReader::Scan(const ScanCallback& callback) {
  for (int i = 0; i < zip_get_num_entries(archive(), 0); ++i) {
    if (auto digest = UnitDigest(zip_get_name(archive(), i, 0))) {
      if (!callback(*digest)) {
        break;
      }
    }
  }
  return OkStatus();
}

}  // namespace kythe
