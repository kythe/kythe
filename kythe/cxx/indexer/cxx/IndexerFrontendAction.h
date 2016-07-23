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

/// \file IndexerFrontendAction.h
/// \brief Defines a tool that passes notifications to a `GraphObserver`.

// This file uses the Clang style conventions.

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <set>
#include <string>
#include <utility>

#include "glog/logging.h"
#include "kythe/cxx/common/cxx_details.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

#include "GraphObserver.h"
#include "IndexerASTHooks.h"
#include "IndexerPPCallbacks.h"

namespace kythe {
namespace proto {
class CompilationUnit;
class FileData;
} // namespace proto
class KytheClaimClient;

/// \brief Runs a given tool on a piece of code with a given assumed filename.
/// \returns true on success, false on failure.
bool RunToolOnCode(std::unique_ptr<clang::FrontendAction> tool_action,
                   llvm::Twine code, const std::string &filename);

// A FrontendAction that extracts information about a translation unit both
// from its AST (using an ASTConsumer) and from preprocessing (with a
// PPCallbacks implementation).
//
// TODO(jdennett): Test/implement/document the rest of this.
//
// TODO(jdennett): Consider moving/renaming this to kythe::ExtractIndexAction.
class IndexerFrontendAction : public clang::ASTFrontendAction {
public:
  IndexerFrontendAction(GraphObserver *GO, const HeaderSearchInfo *Info)
      : Observer(CHECK_NOTNULL(GO)), HeaderConfigValid(Info != nullptr) {
    if (HeaderConfigValid) {
      HeaderConfig = *Info;
    }
    Supports.push_back(llvm::make_unique<GoogleFlagsLibrarySupport>());
  }

  /// \brief Barrel through even if we don't understand part of a program?
  /// \param I The behavior to use when an unimplemented entity is encountered.
  void setIgnoreUnimplemented(BehaviorOnUnimplemented B) {
    IgnoreUnimplemented = B;
  }

  /// \param Visit template instantiations?
  /// \param T The behavior to use for template instantiations.
  void setTemplateMode(BehaviorOnTemplates T) { TemplateMode = T; }

  /// \param Emit all data?
  /// \param V Degree of verbosity.
  void setVerbosity(Verbosity V) { Verbosity = V; }

private:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef Filename) override {
    if (HeaderConfigValid) {
      auto &HeaderSearch = CI.getPreprocessor().getHeaderSearchInfo();
      auto &FileManager = CI.getFileManager();
      std::vector<clang::DirectoryLookup> Lookups;
      unsigned CurrentIdx = 0;
      for (const auto &Path : HeaderConfig.paths) {
        const clang::DirectoryEntry *DirEnt =
            FileManager.getDirectory(Path.path);
        if (DirEnt != nullptr) {
          Lookups.push_back(clang::DirectoryLookup(
              DirEnt, Path.characteristic_kind, Path.is_framework));
          ++CurrentIdx;
        } else {
          // This can happen if a path was included in the HeaderSearchInfo,
          // but no headers were found underneath that path during extraction.
          // We'll prune out that path here.
          if (CurrentIdx < HeaderConfig.angled_dir_idx) {
            --HeaderConfig.angled_dir_idx;
          }
          if (CurrentIdx < HeaderConfig.system_dir_idx) {
            --HeaderConfig.system_dir_idx;
          }
        }
      }
      HeaderSearch.ClearFileInfo();
      HeaderSearch.SetSearchPaths(Lookups, HeaderConfig.angled_dir_idx,
                                  HeaderConfig.system_dir_idx, false);
      HeaderSearch.SetSystemHeaderPrefixes(HeaderConfig.system_prefixes);
    }
    if (Observer) {
      Observer->setSourceManager(&CI.getSourceManager());
      Observer->setLangOptions(&CI.getLangOpts());
      Observer->setPreprocessor(&CI.getPreprocessor());
    }
    return llvm::make_unique<IndexerASTConsumer>(
        Observer, IgnoreUnimplemented, TemplateMode, Verbosity, Supports);
  }

  bool BeginSourceFileAction(clang::CompilerInstance &CI,
                             llvm::StringRef Filename) override {
    if (Observer) {
      CI.getPreprocessor().addPPCallbacks(llvm::make_unique<IndexerPPCallbacks>(
          CI.getPreprocessor(), *Observer, Verbosity));
    }
    CI.getLangOpts().CommentOpts.ParseAllComments = true;
    CI.getLangOpts().RetainCommentsFromSystemHeaders = true;
    return true;
  }

  bool usesPreprocessorOnly() const override { return false; }

  /// The `GraphObserver` used for reporting information.
  GraphObserver *Observer;
  /// Whether to die on missing cases or to continue onward.
  BehaviorOnUnimplemented IgnoreUnimplemented = BehaviorOnUnimplemented::Abort;
  /// Whether to visit template instantiations.
  BehaviorOnTemplates TemplateMode = BehaviorOnTemplates::VisitInstantiations;
  /// Whether to emit all data.
  enum Verbosity Verbosity = kythe::Verbosity::Classic;
  /// Configuration information for header search.
  HeaderSearchInfo HeaderConfig;
  /// Whether to use HeaderConfig.
  bool HeaderConfigValid;
  /// Library-specific callbacks.
  LibrarySupports Supports;
};

/// \brief Allows stdin to be replaced with a mapped file.
///
/// `clang::CompilerInstance::InitializeSourceManager` special-cases the path
/// "-" to llvm::MemoryBuffer::getSTDIN() even if "-" has been remapped.
/// This class mutates the frontend input list such that any file input that
/// would trip this logic instead tries to resolve a file named "<stdin>",
/// which is a token used elsewhere in the compiler to refer to standard input.
class StdinAdjustSingleFrontendActionFactory
    : public clang::tooling::FrontendActionFactory {
  std::unique_ptr<clang::FrontendAction> Action;

public:
  /// \param Action The single FrontendAction to run once. Takes ownership.
  StdinAdjustSingleFrontendActionFactory(
      std::unique_ptr<clang::FrontendAction> Action)
      : Action(std::move(Action)) {}

  bool
  runInvocation(clang::CompilerInvocation *Invocation,
                clang::FileManager *Files,
                std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
                clang::DiagnosticConsumer *DiagConsumer) override {
    auto &FEOpts = Invocation->getFrontendOpts();
    for (auto &Input : FEOpts.Inputs) {
      if (Input.isFile() && Input.getFile() == "-") {
        Input = clang::FrontendInputFile("<stdin>", Input.getKind(),
                                         Input.isSystem());
      }
    }
    return clang::tooling::FrontendActionFactory::runInvocation(
        Invocation, Files, PCHContainerOps, DiagConsumer);
  }

  /// Note that FrontendActionFactory::create() specifies that the
  /// returned action is owned by the caller.
  clang::FrontendAction *create() override { return Action.release(); }
};

/// \brief Options that control how the indexer behaves.
struct IndexerOptions {
  /// \brief The directory to normalize paths against. Must be absolute.
  std::string EffectiveWorkingDirectory = "/";
  /// \brief What to do with template expansions.
  BehaviorOnTemplates TemplateBehavior =
      BehaviorOnTemplates::VisitInstantiations;
  /// \brief What to do when we don't know what to do.
  BehaviorOnUnimplemented UnimplementedBehavior =
      BehaviorOnUnimplemented::Abort;
  /// \brief Whether to emit all data.
  enum Verbosity Verbosity = kythe::Verbosity::Classic;
  /// \brief Enable experimental lossy claiming if true.
  bool EnableLossyClaiming = false;
  /// \brief Whether to allow access to the raw filesystem.
  bool AllowFSAccess = false;
  /// \brief Whether to drop data found to be template instantiation
  /// independent.
  bool DropInstantiationIndependentData = false;
  /// \brief A function that is called as the indexer enters and exits various
  /// phases of execution (in strict LIFO order).
  ProfilingCallback ReportProfileEvent = [](const char *, ProfilingEvent) {};
};

/// \brief Indexes `Unit`, reading from `Files` in the assumed and writing
/// entries to `Output`.
/// \param Unit The CompilationUnit to index
/// \param Files A vector of files to read from. May be modified if the Unit
/// does not contain a proper header search table.
/// \param ClaimClient The claim client to use.
/// \param Cache The hash cache to use, or nullptr if none.
/// \param Output The output stream to use.
/// \param Options Configuration settings for this run.
/// \param MetaSupports Metadata support for this run.
/// \return empty if OK; otherwise, an error description.
std::string IndexCompilationUnit(const proto::CompilationUnit &Unit,
                                 std::vector<proto::FileData> &Files,
                                 KytheClaimClient &ClaimClient,
                                 HashCache *Cache, KytheOutputStream &Output,
                                 const IndexerOptions &Options,
                                 const MetadataSupports *MetaSupports);

} // namespace kythe

#endif // KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_
