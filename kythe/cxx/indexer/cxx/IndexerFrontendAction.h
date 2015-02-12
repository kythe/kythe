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

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

#include "GraphObserver.h"
#include "IndexerASTHooks.h"
#include "IndexerPPCallbacks.h"

namespace kythe {

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
  explicit IndexerFrontendAction(GraphObserver *GO) : Observer(GO) {
    assert(GO != nullptr);
  }

  /// \brief Barrel through even if we don't understand part of a program?
  /// \param I The behavior to use when an unimplemented entity is encountered.
  void setIgnoreUnimplemented(BehaviorOnUnimplemented B) {
    IgnoreUnimplemented = B;
  }

  /// \param Visit template instantiations?
  /// \param T The behavior to use for template instantiations.
  void setTemplateMode(BehaviorOnTemplates T) { TemplateMode = T; }

private:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef Filename) override {
    if (Observer) {
      Observer->setSourceManager(&CI.getSourceManager());
      Observer->setLangOptions(&CI.getLangOpts());
      Observer->setPreprocessor(&CI.getPreprocessor());
    }
    return llvm::make_unique<IndexerASTConsumer>(Observer, IgnoreUnimplemented,
                                                 TemplateMode);
  }

  bool BeginSourceFileAction(clang::CompilerInstance &CI,
                             llvm::StringRef Filename) override {
    if (Observer) {
      CI.getPreprocessor().addPPCallbacks(llvm::make_unique<IndexerPPCallbacks>(
          CI.getPreprocessor(), *Observer));
    }
    return true;
  }

  bool usesPreprocessorOnly() const override { return false; }

  /// The `GraphObserver` used for reporting information.
  GraphObserver *Observer;
  /// Whether to die on missing cases or to continue onward.
  BehaviorOnUnimplemented IgnoreUnimplemented = BehaviorOnUnimplemented::Abort;
  /// Whether to visit template instantiations.
  BehaviorOnTemplates TemplateMode = BehaviorOnTemplates::VisitInstantiations;
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
  clang::FrontendAction *Action;

public:
  /// \param Action The single FrontendAction to run once. Takes ownership.
  StdinAdjustSingleFrontendActionFactory(clang::FrontendAction *Action)
      : Action(Action) {}

  virtual bool runInvocation(clang::CompilerInvocation *Invocation,
                             clang::FileManager *Files,
                             clang::DiagnosticConsumer *DiagConsumer) override {
    auto &FEOpts = Invocation->getFrontendOpts();
    for (auto &Input : FEOpts.Inputs) {
      if (Input.isFile() && Input.getFile() == "-") {
        Input = clang::FrontendInputFile("<stdin>", Input.getKind(),
                                         Input.isSystem());
      }
    }
    return clang::tooling::FrontendActionFactory::runInvocation(
        Invocation, Files, DiagConsumer);
  }

  /// Note that FrontendActionFactory::create() specifies that the
  /// returned action is owned by the caller.
  clang::FrontendAction *create() override { return Action; }
};

} // namespace kythe

#endif // KYTHE_CXX_INDEXER_CXX_INDEXER_FRONTEND_ACTION_H_
