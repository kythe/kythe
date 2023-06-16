/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_PP_CALLBACKS_H_
#define KYTHE_CXX_INDEXER_CXX_PP_CALLBACKS_H_

#include "GraphObserver.h"
#include "IndexerASTHooks.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Token.h"

namespace kythe {

/// \brief Listener for preprocessor events, handling file tracking and macro
/// use and definition.
class IndexerPPCallbacks : public clang::PPCallbacks {
 public:
  IndexerPPCallbacks(clang::Preprocessor& PP, GraphObserver& GO,
                     int UsrByteSize);
  ~IndexerPPCallbacks() override;

  void FileChanged(clang::SourceLocation Loc,
                   PPCallbacks::FileChangeReason Reason,
                   clang::SrcMgr::CharacteristicKind FileType,
                   clang::FileID PrevFID) override;

  void MacroDefined(const clang::Token& Token,
                    const clang::MacroDirective* Macro) override;

  void MacroExpands(const clang::Token& Token,
                    const clang::MacroDefinition& Macro,
                    clang::SourceRange Range,
                    const clang::MacroArgs* Args) override;

  void Defined(const clang::Token& MacroName,
               const clang::MacroDefinition& Macro,
               clang::SourceRange Range) override;

  void Ifdef(clang::SourceLocation Location, const clang::Token& MacroName,
             const clang::MacroDefinition& Macro) override;

  void Ifndef(clang::SourceLocation Location, const clang::Token& MacroName,
              const clang::MacroDefinition& Macro) override;

  void MacroUndefined(const clang::Token& MacroName,
                      const clang::MacroDefinition& Macro,
                      const clang::MacroDirective* Undef) override;

  void InclusionDirective(clang::SourceLocation HashLocation,
                          const clang::Token& IncludeToken,
                          llvm::StringRef Filename, bool IsAngled,
                          clang::CharSourceRange FilenameRange,
                          clang::OptionalFileEntryRef FileRef,
                          llvm::StringRef SearchPath,
                          llvm::StringRef RelativePath,
                          const clang::Module* Imported,
                          clang::SrcMgr::CharacteristicKind FileType) override;

  /// \brief Run by a `clang::PragmaHandler` to handle the `kythe_metadata`
  /// pragma.
  ///
  /// This has the same semantics as `clang::PragmaHandler::HandlePragma`.
  /// We pass Clang a throwaway `PragmaHandler` instance that delegates to
  /// this member function.
  ///
  /// \sa clang::PragmaHandler::HandlePragma
  void HandleKytheMetadataPragma(clang::Preprocessor& Preprocessor,
                                 clang::PragmaIntroducerKind Introducer,
                                 clang::Token& FirstToken);

  /// \brief Run by a `clang::PragmaHandler` to handle the
  /// `kythe_inline_metadata` pragma.
  ///
  /// This has the same semantics as `clang::PragmaHandler::HandlePragma`.
  /// We pass Clang a throwaway `PragmaHandler` instance that delegates to
  /// this member function.
  ///
  /// \sa clang::PragmaHandler::HandlePragma
  void HandleKytheInlineMetadataPragma(clang::Preprocessor& Preprocessor,
                                       clang::PragmaIntroducerKind Introducer,
                                       clang::Token& FirstToken);

  void EndOfMainFile() override;

 private:
  /// Some heuristics (such as whether a macro is a header guard) can only
  /// be determined when a file has been fully preprocessed. A `DeferredRecord`
  /// keeps track of a macro that needs this kind of analysis.
  struct DeferredRecord {
    const clang::Token MacroName;        ///< The spelling site for this macro.
    const clang::MacroDirective* Macro;  ///< The macro itself, if defined.
    bool WasDefined;  ///< If true, the macro was defined at time of deferral.
    GraphObserver::Range Range;  ///< The range covering the spelling site.
  };

  /// \brief Emits the deferred macros that should be emitted according to
  /// heuristics.
  void FilterAndEmitDeferredRecords();

  /// \brief Keeps track of all DeferredRecords we've made.
  std::vector<DeferredRecord> DeferredRecords;

  /// \brief Returns `SR` as a `Range` in the `IndexerPPCallbacks`'s current
  /// RangeContext.
  GraphObserver::Range RangeInCurrentContext(const clang::SourceRange& SR) {
    // TODO(zarko): which expansion are we in? (We don't generally want
    // to record this, though.)
    return GraphObserver::Range(SR, Observer.getClaimTokenForRange(SR));
  }

  /// \brief Records the use of a macro if that macro is defined.
  /// \param MacroNameToken The spelling site of the macro.
  void AddMacroReferenceIfDefined(const clang::Token& MacroNameToken);

  /// \brief Emits a reference to a macro.
  /// \param MacroNameToken The token that spelled out the macro's name.
  /// \param Info The `MacroInfo` best matching `MacroNameToken`.
  /// \param IsDefined true if the macro was defined at time of reference.
  void AddReferenceToMacro(const clang::Token& MacroNameToken,
                           clang::MacroInfo const& Info, bool IsDefined);

  /// \brief Returns the source range of `Token`.
  GraphObserver::Range RangeForTokenInCurrentContext(const clang::Token& Token);

  /// \brief Builds a `NodeId` for some macro.
  /// \param Spelling A token representing the macro's spelling.
  /// \param Info The `MacroInfo` representing the macro.
  GraphObserver::NodeId BuildNodeIdForMacro(const clang::Token& Spelling,
                                            clang::MacroInfo const& Info);

  /// \brief Builds a `NameId` for some macro.
  /// \param Spelling A token representing the macro's spelling.
  GraphObserver::NameId BuildNameIdForMacro(const clang::Token& Spelling);

  /// The location of the hash for the last-seen #include.
  clang::SourceLocation LastInclusionHash;
  /// The `clang::Preprocessor` to which this `IndexerPPCallbacks` is listening.
  const clang::Preprocessor& Preprocessor;
  /// The `GraphObserver` we will use for reporting information.
  GraphObserver& Observer;
  /// \brief The number of (raw) bytes to use to represent a USR. If 0,
  /// no USRs will be recorded.
  int UsrByteSize = 0;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_PP_CALLBACKS_H_
