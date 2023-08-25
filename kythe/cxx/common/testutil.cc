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
#include "kythe/cxx/common/testutil.h"

#include <cstdlib>
#include <string>

#include "absl/log/die_if_null.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"

namespace kythe {
namespace {

// This must match the name from the workspace(name={name})
// rule in the root WORKSPACE file.
constexpr char kDefaultWorkspace[] = "io_kythe";

}  // namespace

std::string TestSourceRoot() {
  const auto* workspace = std::getenv("TEST_WORKSPACE");
  if (workspace == nullptr) {
    workspace = kDefaultWorkspace;
  }
  return absl::StrCat(
      absl::StripSuffix(ABSL_DIE_IF_NULL(std::getenv("TEST_SRCDIR")), "/"), "/",
      absl::StripSuffix(workspace, "/"), "/");
}

}  // namespace kythe
