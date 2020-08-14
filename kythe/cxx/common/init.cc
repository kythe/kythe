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

#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/symbolize.h"
#include "glog/logging.h"

namespace kythe {

void InitializeProgram(const char* argv0) {
  google::InitGoogleLogging(argv0);
  absl::InitializeSymbolizer(argv0);
  absl::InstallFailureSignalHandler({});
}

}  // namespace kythe
