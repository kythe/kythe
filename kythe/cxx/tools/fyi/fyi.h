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

#ifndef KYTHE_CXX_TOOLS_FYI_FYI_H_
#define KYTHE_CXX_TOOLS_FYI_FYI_H_

#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Tooling/Tooling.h"
#include "kythe/cxx/common/net_client.h"

namespace kythe {
namespace fyi {

class Action;
class FileTracker;

/// \brief Creates actions for fyi passes.
class ActionFactory : public clang::tooling::ToolAction {
 public:
  /// \param xrefs A source for cross-references.
  /// \param iterations The maximum number of iterations to try before stopping.
  ActionFactory(std::unique_ptr<XrefsClient> xrefs, size_t iterations);
  ~ActionFactory();

  /// \brief Call before starting the next iteration around the fixpoint.
  /// Decrements the number of iterations remaining.
  /// \pre iterations > 0.
  void BeginNextIteration();

  /// \brief Returns true if the tool should be run again as part of the
  /// fixpoint loop; false if not.
  bool ShouldRunAgain();

  /// \brief Returns the number of iterations we have left to run.
  size_t iterations() const { return iterations_; }

  /// \copydoc clang::tooling::ToolAction::runInvocation
  bool runInvocation(clang::CompilerInvocation *invocation,
                     clang::FileManager *files,
                     clang::DiagnosticConsumer *diagnostics) override;

 private:
  friend class Action;

  /// \brief All of Clang's builtin headers.
  std::vector<std::unique_ptr<llvm::MemoryBuffer>> builtin_headers_;

  /// The client to use to find cross-references.
  std::unique_ptr<XrefsClient> xrefs_;

  /// \brief Try to find a `FileTracker` for the given path.
  /// \param filename the path to search for
  /// \return the old `FileTracker` for `filename`, or a new one.
  FileTracker *GetOrCreateTracker(llvm::StringRef filename);

  /// \brief Use the rewritten data stored in `file_trackers_` and the
  /// builtin headers in `builtin_headers_` as input.
  /// \param resource_dir The resource directory to use when mapping builtins.
  /// \param remapped_buffers A vector of (path, buffer) to fill. It does
  /// not gain ownership of the `llvm::MemoryBuffer` instances.
  void RemapFiles(llvm::StringRef resource_dir,
                  std::vector<std::pair<std::string, llvm::MemoryBuffer *>>
                      *remapped_buffers);

  /// \brief Maps paths to `FileTracker` instances.
  typedef llvm::StringMap<FileTracker *> FileTrackerMap;

  /// Maps full paths (as seen by Clang) to `FileTracker` instances.
  FileTrackerMap file_trackers_;

  /// How many more times should we run the main loop until we give up?
  size_t iterations_;
};

}  // namespace fyi
}  // namespace kythe

#endif  // KYTHE_CXX_TOOLS_FYI_FYI_H_
