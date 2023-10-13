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

#include <zip.h>

#include "absl/status/status.h"
#include "kythe/cxx/common/status.h"

namespace kythe {
namespace libzip {
namespace {

absl::StatusCode GetStatusCode(zip_error_t* error) {
  switch (zip_error_system_type(error)) {
    case ZIP_ET_SYS:
      return ErrnoToStatusCode(zip_error_code_system(error));
    case ZIP_ET_ZLIB:
      return ZlibStatusCode(zip_error_code_system(error));
    case ZIP_ET_NONE:
    default:
      return ZlibStatusCode(zip_error_code_zip(error));
  }
}

}  // namespace

absl::Status Error::ToStatus() { return kythe::libzip::ToStatus(get()); }
absl::Status Error::ToStatus() const {
  // Due to caching in zip_error_strerror, it can't be const so we must copy.
  return Error(*this).ToStatus();
}

absl::Status ToStatus(zip_error_t* error) {
  absl::StatusCode code = GetStatusCode(error);
  if (code == absl::StatusCode::kOk) {
    return absl::OkStatus();
  }
  return absl::Status(code, zip_error_strerror(error));
}

absl::StatusCode ZlibStatusCode(int zlib_error) {
  using absl::StatusCode;
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
