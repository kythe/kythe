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

/// \file IndexerFrontendAction.h
/// \brief Defines a tool that passes notifications to a `GraphObserver`.

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "GraphObserver.h"
#include "IndexerASTHooks.h"
#include "IndexerPPCallbacks.h"
#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "absl/memory/memory.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Tooling/Tooling.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "kythe/cxx/extractor/cxx_details.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "re2/re2.h"

namespace kythe {
namespace proto {
class CompilationUnit;
class FileData;
}  // namespace proto
class KytheClaimClient;

/// \brief Runs a given tool on a piece of code with a given assumed filename.
/// \returns true on success, false on failure.
bool RunToolOnCode(std::unique_ptr<clang::FrontendAction> tool_action,
                   llvm::Twine code, const std::string& filename);

// A FrontendAction that extracts information about a translation unit both
// from its AST (using an ASTConsumer) and from preprocessing (with a
// PPCallbacks implementation).
//
// TODO(jdennett): Test/implement/document the rest of this.
//
// TODO(jdennett): Consider moving/renaming this to kythe::ExtractIndexAction.
class IndexerFrontendAction : public clang::ASTFrontendAction {
 public:
  IndexerFrontendAction(GraphObserver* GO, const HeaderSearchInfo* Info,
                        const LibrarySupports* LibrarySupports,
                        const IndexerOptions& options
                            ABSL_ATTRIBUTE_LIFETIME_BOUND)
      : Observer(ABSL_DIE_IF_NULL(GO)),
        HeaderConfigValid(Info != nullptr),
        Supports(*ABSL_DIE_IF_NULL(LibrarySupports)),
        options_(options) {
    if (HeaderConfigValid) {
      HeaderConfig = *Info;
    }
  }

 protected:
  bool PrepareToExecuteAction(clang::CompilerInstance& CI) override {
    CI.getPreprocessorOpts().DisablePCHOrModuleValidation =
        clang::DisableValidationForModuleKind::All;
    return clang::ASTFrontendAction::PrepareToExecuteAction(CI);
  }

 private:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& CI, llvm::StringRef Filename) override {
    if (HeaderConfigValid) {
      auto& HeaderSearch = CI.getPreprocessor().getHeaderSearchInfo();
      auto& FileManager = CI.getFileManager();
      std::vector<clang::DirectoryLookup> Lookups;
      unsigned CurrentIdx = 0;
      for (const auto& Path : HeaderConfig.paths) {
        auto DirEnt = FileManager.getDirectoryRef(Path.path);
        if (DirEnt) {
          Lookups.push_back(clang::DirectoryLookup(
              DirEnt.get(), Path.characteristic_kind, Path.is_framework));
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
                                  HeaderConfig.system_dir_idx, false,
                                  llvm::DenseMap<unsigned, unsigned>());
      HeaderSearch.SetSystemHeaderPrefixes(HeaderConfig.system_prefixes);
    }
    if (Observer) {
      Observer->setSourceManager(&CI.getSourceManager());
      Observer->setLangOptions(&CI.getLangOpts());
      Observer->setPreprocessor(&CI.getPreprocessor());
    }
    return std::make_unique<IndexerASTConsumer>(Observer, Supports, options_);
  }

  bool BeginSourceFileAction(clang::CompilerInstance& CI) override {
    if (Observer) {
      CI.getPreprocessor().addPPCallbacks(std::make_unique<IndexerPPCallbacks>(
          CI.getPreprocessor(), *Observer, options_.UsrByteSize));
    }
    CI.getLangOpts().CommentOpts.ParseAllComments = true;
    CI.getLangOpts().RetainCommentsFromSystemHeaders = true;
    return true;
  }

  bool usesPreprocessorOnly() const override { return false; }

  /// The `GraphObserver` used for reporting information.
  GraphObserver* Observer;
  /// Configuration information for header search.
  HeaderSearchInfo HeaderConfig;
  /// Whether to use HeaderConfig.
  bool HeaderConfigValid;
  /// Library-specific callbacks.
  const LibrarySupports& Supports;
  /// \brief Options to use when indexing.
  const IndexerOptions& options_;
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

  bool runInvocation(
      std::shared_ptr<clang::CompilerInvocation> Invocation,
      clang::FileManager* Files,
      std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps,
      clang::DiagnosticConsumer* DiagConsumer) override {
    auto& FEOpts = Invocation->getFrontendOpts();
    for (auto& Input : FEOpts.Inputs) {
      if (Input.isFile() && Input.getFile() == "-") {
        Input = clang::FrontendInputFile("<stdin>", Input.getKind(),
                                         Input.isSystem());
      }
    }
    // Disable dependency outputs. The indexer should not write to arbitrary
    // files on its host (as dictated by -MD-style flags).
    Invocation->getDependencyOutputOpts().OutputFile.clear();
    Invocation->getDependencyOutputOpts().HeaderIncludeOutputFile.clear();
    Invocation->getDependencyOutputOpts().DOTOutputFile.clear();
    Invocation->getDependencyOutputOpts().ModuleDependencyOutputDir.clear();
    return clang::tooling::FrontendActionFactory::runInvocation(
        Invocation, Files, PCHContainerOps, DiagConsumer);
  }

  /// Note that FrontendActionFactory::create() specifies that the
  /// returned action is owned by the caller.
  std::unique_ptr<clang::FrontendAction> create() override {
    return std::move(Action);
  }
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
/// \param LibrarySupports Library support for this run.
/// \return empty if OK; otherwise, an error description.
std::string IndexCompilationUnit(
    const proto::CompilationUnit& Unit, std::vector<proto::FileData>& Files,
    KytheClaimClient& ClaimClient, HashCache* Cache, KytheCachingOutput& Output,
    IndexerOptions& Options ABSL_ATTRIBUTE_LIFETIME_BOUND,
    const MetadataSupports* MetaSupports,
    const LibrarySupports* LibrarySupports);

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_
