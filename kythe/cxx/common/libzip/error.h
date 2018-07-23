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

namespace kythe {
namespace libzip {

/// \brief RAII wrapper around zip_error_t.
class Error {
 public:
  Error() { zip_error_init(get()); }
  Error(const Error&) = delete;
  Error& operator=(const Error&) = delete;
  ~Error() { zip_error_fini(get()); }

  zip_error_t* get() { return &error_; }

 private:
  zip_error_t error_;
};


}  // namespace libzip
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_LIBZIP_ERROR_H_
