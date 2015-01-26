/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// This file uses the Clang style conventions.

#include "IndexerFrontendAction.h"

#include <memory>
#include <string>

#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/ADT/Twine.h"

namespace kythe {

bool RunToolOnCode(std::unique_ptr<clang::FrontendAction> tool_action,
                   llvm::Twine code, const std::string &filename) {
  if (tool_action == nullptr)
    return false;
  return clang::tooling::runToolOnCode(tool_action.release(), code, filename);
}

} // namespace kythe
