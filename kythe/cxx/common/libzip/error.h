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

#ifndef KYTHE_CXX_COMMON_LIBZIP_ERROR_H_
#define KYTHE_CXX_COMMON_LIBZIP_ERROR_H_

#include <zip.h>

#include "absl/status/status.h"

namespace kythe {
namespace libzip {

/// \brief RAII wrapper around zip_error_t.
class Error {
 public:
  /// \brief Constructs an Error instance from the libzip error code
  /// and errno, if necessary.
  explicit Error(int code) { zip_error_init_with_code(get(), code); }
  /// \brief Pseudo-copy constructor. Constructs an Error instance copying
  /// the relevant portions of zip_error_t.
  explicit Error(const zip_error_t& error) : Error() {
    zip_error_set(get(), zip_error_code_zip(&error),
                  zip_error_code_system(&error));
  }
  Error() { zip_error_init(get()); }
  Error(const Error& other) : Error(*other.get()) {}
  Error& operator=(const Error& other) {
    zip_error_fini(get());
    zip_error_init(get());
    zip_error_set(get(), zip_error_code_zip(other.get()),
                  zip_error_code_system(other.get()));
    return *this;
  }
  ~Error() { zip_error_fini(get()); }

  /// \brief Converts the Error into a absl::Status.
  absl::Status ToStatus() const;
  absl::Status ToStatus();

  zip_error_t* get() { return &error_; }
  const zip_error_t* get() const { return &error_; }

  int system_type() const { return zip_error_system_type(get()); }
  int zip_code() const { return zip_error_code_zip(get()); }
  int system_code() const { return zip_error_code_system(get()); }

 private:
  zip_error_t error_;
};

/// \brief Converts a zip_error_t into absl::Status.
absl::Status ToStatus(zip_error_t* error);

/// \brief Translates a ZLIB_ER_* constant into a StatusCode.
absl::StatusCode ZlibStatusCode(int zlib_error);

}  // namespace libzip
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_LIBZIP_ERROR_H_
