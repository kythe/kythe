#include "kythe/cxx/common/status.h"

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

}  // namespace kythe
