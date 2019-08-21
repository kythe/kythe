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

#ifndef KYTHE_CXX_COMMON_STATUS_H_
#define KYTHE_CXX_COMMON_STATUS_H_

#include <ostream>
#include <string>

#include "absl/base/attributes.h"
#include "absl/strings/string_view.h"

namespace kythe {

/// \brief Enumeration of possible status codes.
enum class StatusCode {
  kOk = 0,                // No error occurred.
  kCancelled = 1,         // The operation was interrupted.
  kUnknown = 2,           // An otherwise unknown error occurred.
  kInvalidArgument = 3,   // Client specified an invalid argument.
  kDeadlineExceeded = 4,  // Deadline expired before operation could complete.
  kNotFound = 5,          // Some requested entity was not found.
  kAlreadyExists = 6,  // An entity that we attempted to create already exists.
  kPermissionDenied = 7,   // Caller lacks permission to execute the operation.
  kResourceExhausted = 8,  // Some resource (quota, filesystem) is out of space.
  kFailedPrecondition = 9,  // System cannot execute the requested operation.
  kAborted = 10,            // The operation was aborted.
  kOutOfRange = 11,         // Operation attempted beyond the valid range.
  kUnimplemented = 12,      // Requested operation has not been implemented.
  kInternal = 13,           // An unspecified internal error has occured.
  kUnavailable = 14,        // The service is currently unavailable.
  kDataLoss = 15,           // Unrecoverable data loss or corruption.
  kUnauthenticated = 16,    // The caller lacks valid creditentials.
  kDoNotUseReservedForFutureExpansionUseDefaultInSwitchInstead_ = 20,
};

/// \brief A trivial error indicator type, with an API roughly based on
/// tensorflow::Status and grpc::Status.
// TODO(shahms): Replace this in favor of absl::Status when available.
class ABSL_MUST_USE_RESULT Status final {
 public:
  /// \brief An Ok status with no message.
  Status() = default;

  /// \brief A non-Ok status with provided message.
  Status(StatusCode code, absl::string_view message)
      : code_(code), message_(message) {}

  /// \brief Status is both copyable and movable.
  Status(const Status&) = default;
  Status(Status&&) = default;
  Status& operator=(const Status&) = default;
  Status& operator=(Status&&) = default;

  ABSL_MUST_USE_RESULT bool ok() const { return code_ == StatusCode::kOk; }
  StatusCode code() const { return code_; }
  absl::string_view message() const { return message_; }

  /// \brief Returns a human-readable error message.
  std::string ToString() const;

  /// \brief Explicit ignore the error, silencing compiler warnings.
  void IgnoreError() const {}

 private:
  /// \brief Make `Status` appropriately streamable.
  friend std::ostream& operator<<(std::ostream& os, const Status& status) {
    os << status.ToString();
    return os;
  }

  StatusCode code_ = StatusCode::kOk;
  std::string message_;
};

/// \brief Returns a non-error status.
inline Status OkStatus() { return {}; }

/// \brief Return a Cancelled error with the given message.
Status CancelledError(absl::string_view message);
/// \brief Return an Unknown error with the given message.
Status UnknownError(absl::string_view message);
/// \brief Return an InvalidArgument error with the given message.
Status InvalidArgumentError(absl::string_view message);
/// \brief Return a DeadlineExceeded error with the given message.
Status DeadlineExceededError(absl::string_view message);
/// \brief Return a NotFound error with the given message.
Status NotFoundError(absl::string_view message);
/// \brief Return an AlreadyExists error with the given message.
Status AlreadyExistsError(absl::string_view message);
/// \brief Return a PermissionDenied error with the given message.
Status PermissionDeniedError(absl::string_view message);
/// \brief Return a ResourceExhausted error with the given message.
Status ResourceExhaustedError(absl::string_view message);
/// \brief Return a FailedPrecondition error with the given message.
Status FailedPreconditionError(absl::string_view message);
/// \brief Return an Aborted error with the given message.
Status AbortedError(absl::string_view message);
/// \brief Return an OutOfRange error with the given message.
Status OutOfRangeError(absl::string_view message);
/// \brief Return an Unimplemented error with the given message.
Status UnimplementedError(absl::string_view message);
/// \brief Return an Internal error with the given message.
Status InternalError(absl::string_view message);
/// \brief Return an Unavailable error with the given message.
Status UnavailableError(absl::string_view message);
/// \brief Return a DataLoss error with the given message.
Status DataLossError(absl::string_view message);
/// \brief Return an Unauthenticated error with the given message.
Status UnauthenticatedError(absl::string_view message);

/// \brief Return an error converted from a POSIX errno value.
StatusCode ErrnoToStatusCode(int error_number);
Status ErrnoToStatus(int error_number);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_STATUS_H_
