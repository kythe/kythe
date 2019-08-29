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

#include "kythe/cxx/common/status.h"

#include <errno.h>

#include <type_traits>

#include "absl/strings/str_cat.h"
#include "glog/logging.h"

namespace kythe {
namespace {

template <typename T, typename U = typename std::underlying_type<T>::type>
U UnderlyingCast(T t) {
  return static_cast<U>(t);
}

absl::string_view AsString(StatusCode code) {
  switch (code) {
    case StatusCode::kOk:
      return "OK";
    case StatusCode::kCancelled:
      return "CANCELLED";
    case StatusCode::kUnknown:
      return "UNKNOWN";
    case StatusCode::kInvalidArgument:
      return "INVALID_ARGUMENT";
    case StatusCode::kDeadlineExceeded:
      return "DEADLINE_EXCEEDED";
    case StatusCode::kNotFound:
      return "NOT_FOUND";
    case StatusCode::kAlreadyExists:
      return "ALREADY_EXISTS";
    case StatusCode::kPermissionDenied:
      return "PERMISSION_DENIED";
    case StatusCode::kResourceExhausted:
      return "RESOURCE_EXHAUSTED";
    case StatusCode::kFailedPrecondition:
      return "FAILED_PRECONDITION";
    case StatusCode::kAborted:
      return "ABORTED";
    case StatusCode::kOutOfRange:
      return "OUT_OF_RANGE";
    case StatusCode::kUnimplemented:
      return "UNIMPLEMENTED";
    case StatusCode::kInternal:
      return "INTERNAL";
    case StatusCode::kUnavailable:
      return "UNAVAILABLE";
    case StatusCode::kDataLoss:
      return "DATA_LOSS";
    case StatusCode::kUnauthenticated:
      return "UNAUTHENTICATED";
    case StatusCode::
        kDoNotUseReservedForFutureExpansionUseDefaultInSwitchInstead_:
      LOG(DFATAL) << "Reserved status code!";
      return "(reserved)";
  }
  LOG(DFATAL) << "Unknown StatusCode: " << UnderlyingCast(code);
  return "(invalid)";
}

}  // namespace

std::string Status::ToString() const {
  return absl::StrCat(AsString(code_), ": ", message_);
}

Status CancelledError(absl::string_view message) {
  return {StatusCode::kCancelled, std::string(message)};
}
Status UnknownError(absl::string_view message) {
  return {StatusCode::kUnknown, std::string(message)};
}
Status InvalidArgumentError(absl::string_view message) {
  return {StatusCode::kInvalidArgument, std::string(message)};
}
Status DeadlineExceededError(absl::string_view message) {
  return {StatusCode::kDeadlineExceeded, std::string(message)};
}
Status NotFoundError(absl::string_view message) {
  return {StatusCode::kNotFound, std::string(message)};
}
Status AlreadyExistsError(absl::string_view message) {
  return {StatusCode::kAlreadyExists, std::string(message)};
}
Status PermissionDeniedError(absl::string_view message) {
  return {StatusCode::kPermissionDenied, std::string(message)};
}
Status ResourceExhaustedError(absl::string_view message) {
  return {StatusCode::kResourceExhausted, std::string(message)};
}
Status FailedPreconditionError(absl::string_view message) {
  return {StatusCode::kFailedPrecondition, std::string(message)};
}
Status AbortedError(absl::string_view message) {
  return {StatusCode::kAborted, std::string(message)};
}
Status OutOfRangeError(absl::string_view message) {
  return {StatusCode::kOutOfRange, std::string(message)};
}
Status UnimplementedError(absl::string_view message) {
  return {StatusCode::kUnimplemented, std::string(message)};
}
Status InternalError(absl::string_view message) {
  return {StatusCode::kInternal, std::string(message)};
}
Status UnavailableError(absl::string_view message) {
  return {StatusCode::kUnavailable, std::string(message)};
}
Status DataLossError(absl::string_view message) {
  return {StatusCode::kDataLoss, std::string(message)};
}
Status UnauthenticatedError(absl::string_view message) {
  return {StatusCode::kUnauthenticated, std::string(message)};
}

StatusCode ErrnoToStatusCode(int error_number) {
  switch (error_number) {
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
    case ENODEV:  // No such device
    case ENOENT:  // No such file or directory
    case ENXIO:   // No such device or address
    case ESRCH:   // No such process
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

Status ErrnoToStatus(int error_number) {
  return Status(ErrnoToStatusCode(error_number), strerror(error_number));
}

}  // namespace kythe
