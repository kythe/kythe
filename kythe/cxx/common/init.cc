/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
#include "kythe/cxx/common/init.h"

#include <cassert>
#include <cstring>

#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/symbolize.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

namespace kythe {

#if defined(KYTHE_OVERRIDE_ASSERT_FAIL)
extern "C" {

#ifndef __THROW
#define __THROW throw()
#endif

// Make sure we get a legible stack trace when assert from <assert.h> fails.
// This works around https://github.com/abseil/abseil-cpp/issues/769.
//
// We are replacing implementation-defined function (defined in glibc)
// used by the expansion of system defined assert() macro.
void __assert_fail(const char* assertion, const char* file, unsigned int line,
                   const char* function) __THROW {
  LOG(FATAL) << "assert.h assertion failed at " << file << ":" << line
             << (function ? " in " : "") << (function ? function : "") << ": "
             << assertion;
}

// Same as above, but for assert_perror.
void __assert_perror_fail(int errnum, const char* file, unsigned int line,
                          const char* function) __THROW {
  LOG(FATAL) << "assert.h assert_perror failed at " << file << ":" << line
             << " " << (function ? function : "") << ": "
             << std::strerror(errnum) << " [" << errnum << "]";
}
}  // extern "C"
#endif

void InitializeProgram(const char* argv0) {
  absl::InitializeLog();
  absl::InitializeSymbolizer(argv0);
  absl::InstallFailureSignalHandler({});
}

}  // namespace kythe
