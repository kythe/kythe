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

#include "IndexerPPCallbacks.h"
#include "GraphObserver.h"

#include "glog/logging.h"
#include "kythe/cxx/common/path_utils.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

// TODO(zarko): IndexerASTHooks::RangeInCurrentContext should query the macro
// context. IndexerPPCallbacks doesn't need to query for the template context,
// since we'll always be in a state that looks like
//   /macro-expansion* template-inst*/,
// as template processing happens at a later stage.

// Embedding non-file source locations into vnames generates names that are
// not stable wrt include ordering (there appears to be a single scratch
// buffer with a line counter that is incrementally bumped).
// TODO(zarko): Fix this by using a hash of the scratch buffer + the file
// location into which the scratch buffer is being inlined. We might also be
// able to hook these locations to the macros that generate them, allowing us
// to insert edges for token pastes.

namespace kythe {

IndexerPPCallbacks::IndexerPPCallbacks(clang::Preprocessor &PP,
                                       GraphObserver &GO, enum Verbosity V)
    : Preprocessor(PP), Observer(GO), Verbosity(V) {
  class MetadataPragmaHandlerWrapper : public clang::PragmaHandler {
  public:
    MetadataPragmaHandlerWrapper(IndexerPPCallbacks *context)
        : PragmaHandler("kythe_metadata"), context_(context) {}
    void HandlePragma(clang::Preprocessor &Preprocessor,
                      clang::PragmaIntroducerKind Introducer,
                      clang::Token &FirstToken) override {
      context_->HandleKytheMetadataPragma(Preprocessor, Introducer, FirstToken);
    }

  private:
    IndexerPPCallbacks *context_;
  };
  // Clang takes ownership.
  PP.AddPragmaHandler(new MetadataPragmaHandlerWrapper(this));
}

IndexerPPCallbacks::~IndexerPPCallbacks() {}

void IndexerPPCallbacks::FileChanged(clang::SourceLocation Loc,
                                     PPCallbacks::FileChangeReason Reason,
                                     clang::SrcMgr::CharacteristicKind FileType,
                                     clang::FileID PrevFID) {
  switch (Reason) {
  case clang::PPCallbacks::EnterFile:
    Observer.pushFile(LastInclusionHash, Loc);
    break;
  case clang::PPCallbacks::ExitFile:
    Observer.popFile();
    break;
  case clang::PPCallbacks::SystemHeaderPragma:
    break;
  // RenameFile occurs when a #line directive is encountered, for example:
  // #line 10 "foo.cc"
  case clang::PPCallbacks::RenameFile:
    break;
  default:
    llvm::dbgs() << "Unknown FileChangeReason " << Reason << "\n";
  }
}

void IndexerPPCallbacks::FilterAndEmitDeferredRecords() {
  for (const auto &R : DeferredRecords) {
    if (R.WasDefined) {
      if (!R.Macro->getMacroInfo()->isUsedForHeaderGuard()) {
        Observer.recordBoundQueryRange(
            R.Range,
            BuildNodeIdForMacro(R.MacroName, *R.Macro->getMacroInfo()));
      }
    } else {
      Observer.recordUnboundQueryRange(R.Range,
                                       BuildNameIdForMacro(R.MacroName));
    }
  }
  DeferredRecords.clear();
}

void IndexerPPCallbacks::EndOfMainFile() {
  FilterAndEmitDeferredRecords();
  Observer.popFile();
}

GraphObserver::Range
IndexerPPCallbacks::RangeForTokenInCurrentContext(const clang::Token &Token) {
  const auto Start = Token.getLocation();
  const auto End = Start.getLocWithOffset(Token.getLength());
  return RangeInCurrentContext(clang::SourceRange(Start, End));
}

void IndexerPPCallbacks::MacroDefined(const clang::Token &Token,
                                      const clang::MacroDirective *Macro) {
  if (Macro == nullptr) {
    return;
  }
  const clang::MacroInfo &Info = *Macro->getMacroInfo();
  clang::FileID MacroFileID =
      Observer.getSourceManager()->getFileID(Info.getDefinitionLoc());
  const clang::FileEntry *MacroFileEntry =
      Observer.getSourceManager()->getFileEntryForID(MacroFileID);
  if (MacroFileEntry == nullptr) {
    // This is a builtin macro. Ignore it.
    return;
  }
  GraphObserver::NodeId MacroId = BuildNodeIdForMacro(Token, Info);
  if (Observer.claimNode(MacroId)) {
    GraphObserver::NameId MacroName = BuildNameIdForMacro(Token);
    Observer.recordDefinitionBindingRange(RangeForTokenInCurrentContext(Token),
                                          MacroId);
    Observer.recordMacroNode(MacroId);
    Observer.recordNamedEdge(MacroId, MacroName);
  }
  // TODO(zarko): Record information about the definition (like other macro
  // references).
}

void IndexerPPCallbacks::MacroUndefined(const clang::Token &MacroName,
                                        const clang::MacroDefinition &Macro) {
  if (!Macro) {
    return;
  }
  const clang::MacroInfo &Info = *Macro.getMacroInfo();
  GraphObserver::NodeId MacroId = BuildNodeIdForMacro(MacroName, Info);
  Observer.recordUndefinesRange(RangeForTokenInCurrentContext(MacroName),
                                MacroId);
}

void IndexerPPCallbacks::MacroExpands(const clang::Token &Token,
                                      const clang::MacroDefinition &Macro,
                                      clang::SourceRange Range,
                                      const clang::MacroArgs *Args) {
  if (!Macro || Range.isInvalid()) {
    return;
  }

  const clang::MacroInfo &Info = *Macro.getMacroInfo();
  GraphObserver::NodeId MacroId = BuildNodeIdForMacro(Token, Info);
  if (!Range.getBegin().isFileID() || !Range.getEnd().isFileID()) {
    if (Verbosity) {
      auto NewBegin =
          Observer.getSourceManager()->getExpansionLoc(Range.getBegin());
      if (!NewBegin.isFileID()) {
        return;
      }
      Range = clang::SourceRange(NewBegin,
                                 clang::Lexer::getLocForEndOfToken(
                                     NewBegin, 0, /* offset from token end */
                                     *Observer.getSourceManager(),
                                     *Observer.getLangOptions()));
      if (Range.isInvalid()) {
        return;
      }
      Observer.recordIndirectlyExpandsRange(RangeInCurrentContext(Range),
                                            MacroId);
    }
  } else {
    Observer.recordExpandsRange(RangeForTokenInCurrentContext(Token), MacroId);
  }
  // TODO(zarko): Index macro arguments.
}

void IndexerPPCallbacks::Defined(const clang::Token &MacroName,
                                 const clang::MacroDefinition &Macro,
                                 clang::SourceRange Range) {
  DeferredRecords.push_back(
      DeferredRecord{MacroName, Macro ? Macro.getLocalDirective() : nullptr,
                     Macro && Macro.getLocalDirective()->isDefined(),
                     RangeForTokenInCurrentContext(MacroName)});
}

void IndexerPPCallbacks::Ifdef(clang::SourceLocation Location,
                               const clang::Token &MacroName,
                               const clang::MacroDefinition &Macro) {
  // Just delegate.
  Defined(MacroName, Macro, clang::SourceRange(Location));
}

void IndexerPPCallbacks::Ifndef(clang::SourceLocation Location,
                                const clang::Token &MacroName,
                                const clang::MacroDefinition &Macro) {
  // Just delegate.
  Defined(MacroName, Macro, clang::SourceRange(Location));
}

void IndexerPPCallbacks::InclusionDirective(
    clang::SourceLocation HashLocation, const clang::Token &IncludeToken,
    llvm::StringRef Filename, bool IsAngled,
    clang::CharSourceRange FilenameRange, const clang::FileEntry *FileEntry,
    llvm::StringRef SearchPath, llvm::StringRef RelativePath,
    const clang::Module *Imported) {
  // TODO(zarko) (Modules): Check if `Imported` is non-null; if so, this
  // was transformed to a module import.
  if (FileEntry != nullptr) {
    Observer.recordIncludesRange(
        RangeInCurrentContext(FilenameRange.getAsRange()), FileEntry);
  }
  LastInclusionHash = HashLocation;
}

void IndexerPPCallbacks::AddMacroReferenceIfDefined(
    const clang::Token &MacroNameToken) {
  if (clang::IdentifierInfo *const NameII =
          MacroNameToken.getIdentifierInfo()) {
    if (NameII->hasMacroDefinition()) {
      if (auto *Macro = Preprocessor.getLocalMacroDirective(NameII)) {
        AddReferenceToMacro(MacroNameToken, *Macro->getMacroInfo(),
                            Macro->isDefined());
      }
    } else if (NameII->hadMacroDefinition()) {
      if (auto *Macro = Preprocessor.getLocalMacroDirectiveHistory(NameII)) {
        AddReferenceToMacro(MacroNameToken, *Macro->getMacroInfo(),
                            Macro->isDefined());
      }
    }
  } else {
    // This shouldn't happen; it's an error to do "defined TOKEN" unless
    // the preprocessor token is an identifier, in which case it should
    // have IdentifierInfo associated with it.  Note that Clang assigns
    // tok::tk_XXXX token types to preprocessing identifier tokens if
    // they have the names of tokens, rather than following the Standard's
    // formal phases of translation which would have them all be just
    // identifiers during preprocessing.
    LOG(FATAL) << "No IdentifierInfo in AddMacroReferenceIfDefined.";
  }
}

void IndexerPPCallbacks::AddReferenceToMacro(const clang::Token &MacroNameToken,
                                             clang::MacroInfo const &Info,
                                             bool IsDefined) {
  llvm::StringRef MacroName(MacroNameToken.getIdentifierInfo()->getName());
  Observer.recordExpandsRange(RangeForTokenInCurrentContext(MacroNameToken),
                              BuildNodeIdForMacro(MacroNameToken, Info));
}

GraphObserver::NameId
IndexerPPCallbacks::BuildNameIdForMacro(const clang::Token &Spelling) {
  CHECK(Spelling.getIdentifierInfo()) << "Macro spelling lacks IdentifierInfo";
  GraphObserver::NameId Id;
  Id.EqClass = GraphObserver::NameId::NameEqClass::Macro;
  Id.Path = Spelling.getIdentifierInfo()->getName();
  return Id;
}

GraphObserver::NodeId
IndexerPPCallbacks::BuildNodeIdForMacro(const clang::Token &Spelling,
                                        clang::MacroInfo const &Info) {
  // Macro definitions always appear at the topmost level *and* always appear
  // in source text (or are implicit). For this reason, it's safe to use
  // location information to stably unique them. However, we must be careful
  // to select canonical paths.
  clang::SourceLocation Loc = Info.getDefinitionLoc();
  std::string IdString;
  llvm::raw_string_ostream Ostream(IdString);
  Ostream << BuildNameIdForMacro(Spelling);
  auto &SM = *Observer.getSourceManager();
  if (Loc.isInvalid()) {
    Ostream << "@invalid";
  } else if (!Loc.isFileID()) {
    // This case shouldn't happen (given the above), but let's be robust.
    llvm::errs() << "Macro definition found in non-file "
                 << Loc.printToString(SM) << "\n";
    Loc.print(Ostream, SM);
  } else if (SM.getFileID(Loc) ==
             Observer.getPreprocessor()->getPredefinesFileID()) {
    // Locations of predefines in the predefine buffer can spuriously differ
    // from TU to TU, so we collapse them here.
    Ostream << "@builtin";
  } else {
    // Remember that we're inheriting the claim token (which in non-trivial
    // cases should contain canonical source file information).
    Ostream << "@" << SM.getFileOffset(Loc);
  }
  return GraphObserver::NodeId(Observer.getClaimTokenForLocation(Loc),
                               Ostream.str());
}

void IndexerPPCallbacks::HandleKytheMetadataPragma(
    clang::Preprocessor &preprocessor, clang::PragmaIntroducerKind introducer,
    clang::Token &first_token) {
  llvm::SmallString<1024> search_path;
  llvm::SmallString<1024> relative_path;
  llvm::SmallString<1024> filename;
  const auto *file = LookupFileForIncludePragma(&preprocessor, &search_path,
                                                &relative_path, &filename);
  if (!file) {
    fprintf(stderr, "Missing metadata file: %s\n", filename.c_str());
    return;
  }
  clang::FileID pragma_file_id =
      Observer.getSourceManager()->getFileID(first_token.getLocation());
  if (!pragma_file_id.isInvalid()) {
    Observer.applyMetadataFile(pragma_file_id, file);
  } else {
    fprintf(stderr, "Metadata pragma was in an impossible place\n");
  }
}

} // namespace kythe
