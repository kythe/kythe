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

#include "kythe/cxx/common/libzip/error.h"

#include <errno.h>

namespace kythe {
namespace libzip {
namespace {

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
    case ENXIO:      // No such device or address
    case ESRCH:      // No such process
      return StatusCode::kNotFound;
    case EEXIST:         // File exists
    case EADDRNOTAVAIL:  // Address not available
    case EALREADY:       // Connection already in progress
      return StatusCode::kAlreadyExists;
    case EPERM:   // Operation not permitted
    case EACCES:  // Permission denied
    case EROFS:   // Read only file system
      return StatusCode::kPermissionDenied;
    case ENOTEMPTY:   // Directory not empty
    case EISDIR:      // Is a directory
    case ENOTDIR:     // Not a directory
    case EADDRINUSE:  // Address already in use
    case EBADF:       // Invalid file descriptor
    case EBUSY:       // Device or resource busy
    case ECHILD:      // No child processes
    case EISCONN:     // Socket is connected
    case ENOTBLK:     // Block device required
    case ENOTCONN:    // The socket is not connected
    case EPIPE:       // Broken pipe
    case ESHUTDOWN:   // Cannot send after transport endpoint shutdown
    case ETXTBSY:     // Text file busy
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
    case EFBIG:      // File too large
    case EOVERFLOW:  // Value too large to be stored in data type
    case ERANGE:     // Result too large
      return StatusCode::kOutOfRange;
    case ENOSYS:           // Function not implemented
    case ENOTSUP:          // Operation not supported
    case EAFNOSUPPORT:     // Address family not supported
    case EPFNOSUPPORT:     // Protocol family not supported
    case EPROTONOSUPPORT:  // Protocol not supported
    case ESOCKTNOSUPPORT:  // Socket type not supported
    case EXDEV:            // Improper link
      return StatusCode::kUnimplemented;
    case EAGAIN:        // Resource temporarily unavailable
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

}  // namespace

Status Error::ToStatus() { return kythe::libzip::ToStatus(get()); }
Status Error::ToStatus() const {
  // Due to caching in zip_error_strerror, it can't be const so we must copy.
  return Error(*this).ToStatus();
}

Status ToStatus(zip_error_t* error) {
  StatusCode code = GetStatusCode(error);
  if (code == StatusCode::kOk) {
    return OkStatus();
  }
  return Status(code, zip_error_strerror(error));
}

StatusCode ZlibStatusCode(int zlib_error) {
  switch (zlib_error) {
    case ZIP_ER_OK:  // No error
      return StatusCode::kOk;
    case ZIP_ER_MULTIDISK:    // Multi-disk zip archives not supported
    case ZIP_ER_COMPNOTSUPP:  // Compression method not supported
    case ZIP_ER_ENCRNOTSUPP:  // Encryption method not supported
    case ZIP_ER_OPNOTSUPP:    // Operation not supported
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

}  // namespace libzip
}  // namespace kythe
