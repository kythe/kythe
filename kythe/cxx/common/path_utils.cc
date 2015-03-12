/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Path.h"
#include "path_utils.h"

namespace kythe {

static void LexicallyEliminateRelativePathNodes(
    llvm::SmallString<1024>* out_path, llvm::StringRef path) {
  std::vector<llvm::StringRef> path_components;
  int skip_count = 0;
  for (auto node = llvm::sys::path::rbegin(path),
            node_end = llvm::sys::path::rend(path);
       node != node_end; ++node) {
    if (*node == "..") {
      ++skip_count;
    } else if (*node != ".") {
      if (skip_count > 0) {
        --skip_count;
      } else {
        path_components.push_back(*node);
      }
    }
  }
  for (auto node = path_components.crbegin(),
            node_end = path_components.crend();
       node != node_end; ++node) {
    llvm::sys::path::append(*out_path, *node);
  }
}

std::string CleanPath(llvm::StringRef in_path) {
  std::string root_part = (llvm::sys::path::root_name(in_path) +
                           llvm::sys::path::root_directory(in_path)).str();
  llvm::SmallString<1024> out_path = llvm::StringRef(root_part);
  LexicallyEliminateRelativePathNodes(&out_path, llvm::StringRef(in_path));
  return out_path.str();
}

std::string JoinPath(llvm::StringRef a, llvm::StringRef b) {
  llvm::SmallString<1024> out_path = a;
  llvm::sys::path::append(out_path, b);
  return out_path.str();
}

std::string MakeCleanAbsolutePath(const std::string& in_path) {
  std::string abs_path = clang::tooling::getAbsolutePath(in_path);
  return CleanPath(abs_path);
}

std::string RelativizePath(const std::string& to_relativize,
                           const std::string& relativize_against) {
  std::string to_relativize_abs = MakeCleanAbsolutePath(to_relativize);
  std::string relativize_against_abs =
      MakeCleanAbsolutePath(relativize_against);
  llvm::StringRef to_relativize_parent =
      llvm::sys::path::parent_path(to_relativize_abs);
  std::string ret =
      to_relativize_parent.startswith(relativize_against_abs)
          ? to_relativize_abs.substr(relativize_against_abs.size() +
                                     llvm::sys::path::get_separator().size())
          : to_relativize_abs;
  return ret;
}

}  // namespace kythe
