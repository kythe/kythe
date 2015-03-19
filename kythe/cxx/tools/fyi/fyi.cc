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

#include "fyi.h"

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/ExternalSemaSource.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "third_party/llvm/src/clang_builtin_headers.h"

namespace kythe {
namespace fyi {

/// \brief Tracks changes and edits to a single file identified by its full
/// path.
///
/// Holds a `llvm::MemoryBuffer` with the results of the most recent edits
/// if edits have been made.
///
/// This object is first created when the compiler enters a new main source
/// file. Before each new compile or reparse pass, the outer loop should call
/// ::BeginPass(). FileTracker is then notified of various events involving the
/// file being processed. If the FileTracker enters a state that is not kBusy,
/// it can make no further progress. Otherwise, when the compile completes, it
/// is expected that the outer loop will attempt a call to ::Rewrite() with a
/// fresh Rewriter instance. If that succeeds, then ::CommitRewrite will update
/// internal buffers for subsequent passes.
class FileTracker {
 public:
  explicit FileTracker(llvm::StringRef filename) : filename_(filename) {}

  /// \brief Returns the current rewritten file (or null, if rewriting hasn't
  /// happend).
  ///
  /// The object shares its lifetime with this FileTracker.
  llvm::MemoryBuffer *memory_buffer() { return memory_buffer_.get(); }

  llvm::StringRef filename() { return filename_; }

  const llvm::StringRef backing_store() {
    const char *start = memory_buffer_backing_store_.data();
    // Why the - 1?: MemoryBufferBackingStore ends with a NUL terminator.
    return llvm::StringRef(start, memory_buffer_backing_store_.size() - 1);
  }

  /// \param Start a new pass involving this `FileTracker`
  void BeginPass() { file_begin_ = clang::SourceLocation(); }

  /// \brief Rewrite the associated source file with our tentative suggestions.
  /// \param rewriter a valid Rewriter.
  /// \return true if changes will be made, false otherwise.
  bool Rewrite(clang::Rewriter *rewriter) {
    /// TODO(zarko): We add an invalid directive here to check to see whether
    /// the rewriting machinery is working. In the final version of this tool,
    /// we will add new #includes.
    rewriter->InsertTextAfter(file_begin_, "#garbage\n");
    return true;
  }

  /// \brief Rewrite the old file into a new file, discarding any previously
  /// allocated buffers.
  /// \param file_id the current ID of the file we are rewriting
  /// \param rewriter a valid Rewriter.
  void CommitRewrite(clang::FileID file_id, clang::Rewriter *rewriter) {
    const clang::RewriteBuffer *buffer = rewriter->getRewriteBufferFor(file_id);
    memory_buffer_backing_store_.clear();
    llvm::raw_svector_ostream buffer_stream(memory_buffer_backing_store_);
    buffer->write(buffer_stream);
    buffer_stream.flush();
    // Required null terminator.
    memory_buffer_backing_store_.push_back(0);
    const char *start = memory_buffer_backing_store_.data();
    llvm::StringRef data(start, memory_buffer_backing_store_.size() - 1);
    memory_buffer_ = llvm::MemoryBuffer::getMemBuffer(data);
  }

  /// \brief Analysis state, maintained across passes.
  enum class State {
    kBusy,     ///< We are trying to repair this file.
    kSuccess,  ///< We have repaired this file.
    kFailure   ///< We are no longer trying to repair this file.
  };

  /// \brief Gets the state (busy, OK, or bad) of this FileTracker.
  State state() const { return state_; }

  /// \brief Marks that this FileTracker cannot be repaired.
  void mark_failed() { state_ = State::kFailure; }

  /// \brief Gets the location at the very top of the file (in this pass).
  clang::SourceLocation file_begin() const { return file_begin_; }

  /// \brief Sets the location at the very top of the file (in this pass).
  void set_file_begin(clang::SourceLocation location) {
    file_begin_ = location;
  }

 private:
  /// The absolute path to the file this FileTracker tracks. Used as a key
  /// to connect between passes.
  std::string filename_;

  /// The location of the beginning of the tracked file. This changes after
  /// each pass.
  clang::SourceLocation file_begin_;

  /// If this file has been modified, points to a MemoryBuffer containing
  /// the full text of the modified file.
  std::unique_ptr<llvm::MemoryBuffer> memory_buffer_ = nullptr;

  /// Data backing the MemoryBuffer.
  llvm::SmallVector<char, 128> memory_buffer_backing_store_;

  /// The current of this FileTracker independent of pass.
  State state_ = State::kBusy;
};

/// \brief During non-reparse passes, PreprocessorHooks listens for events
/// indicating the files being analyzed and their preprocessor directives.
class PreprocessorHooks : public clang::PPCallbacks {
 public:
  /// \param enclosing_pass The `Action` controlling this pass. Not owned.
  explicit PreprocessorHooks(Action *enclosing_pass)
      : enclosing_pass_(enclosing_pass), tracked_file_(nullptr) {}

  /// \copydoc PPCallbacks::FileChanged
  ///
  /// Finds the `FileEntry` and starting `SourceLocation` for each tracked
  /// file on every pass.
  void FileChanged(clang::SourceLocation loc,
                   clang::PPCallbacks::FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind file_type,
                   clang::FileID prev_fid) override;

 private:
  friend class Action;

  /// The current `Action`. Not owned.
  Action *enclosing_pass_;

  /// The `FileEntry` corresponding to the tracker in `enclosing_pass_`.
  /// Not owned.
  const clang::FileEntry *tracked_file_;
};

/// \brief Manages a full parse and any subsequent reparses for a single file.
class Action : public clang::ASTFrontendAction,
               public clang::ExternalSemaSource {
 public:
  explicit Action(ActionFactory &factory) : factory_(factory) {}

  /// \copydoc ASTFrontendAction::BeginInvocation
  bool BeginInvocation(clang::CompilerInstance &CI) override {
    auto *pp_opts = &CI.getPreprocessorOpts();
    pp_opts->RetainRemappedFileBuffers = true;
    pp_opts->AllowPCHWithCompilerErrors = true;
    factory_.RemapFiles(CI.getHeaderSearchOpts().ResourceDir,
                        &pp_opts->RemappedFileBuffers);
    return true;
  }

  /// \copydoc ASTFrontendAction::CreateASTConsumer
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &compiler, llvm::StringRef in_file) override {
    tracker_ = factory_.GetOrCreateTracker(in_file);
    // Don't bother starting a new pass if the tracker is finished.
    if (tracker_->state() == FileTracker::State::kBusy) {
      tracker_->BeginPass();
      compiler.getPreprocessor().addPPCallbacks(
          llvm::make_unique<PreprocessorHooks>(this));
    }
    return llvm::make_unique<clang::ASTConsumer>();
  }

  /// \copydoc ASTFrontendAction::ExecuteAction
  void ExecuteAction() override {
    // We have to reproduce what ASTFrontendAction::ExecuteAction does, since
    // we have to attach ourselves as an ExternalSemaSource to Sema before
    // calling ParseAST.

    // Do nothing if we've already given up on or finished this file.
    if (tracker_->state() != FileTracker::State::kBusy) {
      return;
    }

    clang::CompilerInstance *compiler = &getCompilerInstance();
    assert(!compiler->hasSema() && "CI already has Sema");

    if (hasCodeCompletionSupport() &&
        !compiler->getFrontendOpts().CodeCompletionAt.FileName.empty())
      compiler->createCodeCompletionConsumer();

    clang::CodeCompleteConsumer *completion_consumer = nullptr;
    if (compiler->hasCodeCompletionConsumer())
      completion_consumer = &compiler->getCodeCompletionConsumer();

    compiler->createSema(getTranslationUnitKind(), completion_consumer);
    compiler->getSema().addExternalSource(this);

    // TODO(zarko): Check to see whether we've solved the problem by inspecting
    // the return value here and/or diagnostics. If we have, the file should be
    // marked having state kSuccess.
    clang::ParseAST(compiler->getSema(), compiler->getFrontendOpts().ShowStats,
                    compiler->getFrontendOpts().SkipFunctionBodies);

    auto *source_manager = &compiler->getSourceManager();
    clang::Rewriter rewriter(*source_manager, compiler->getLangOpts());
    clang::FileID file_id = source_manager->getFileID(tracker_->file_begin());

    bool would_change_file = false;
    if (factory_.iterations() != 0) {
      // If there are iterations remaining, try to rewrite the file. Otherwise
      // there would be no point as we would not be able to check whether the
      // rewrites are correct.
      would_change_file = tracker_->Rewrite(&rewriter);
    }

    if (!would_change_file) {
      // There's no chance of anything happening the next time around. Just
      // fail the tracker.
      tracker_->mark_failed();
    } else {
      // Commit changes to the file. (This renders any pointers into the old
      // buffer invalid--so be careful not to save diagnostics.)
      tracker_->CommitRewrite(file_id, &rewriter);
    }
  }

  /// \copydoc ExternalSemaSource::CorrectTypo
  clang::TypoCorrection CorrectTypo(
      const clang::DeclarationNameInfo &Typo, int LookupKind, clang::Scope *S,
      clang::CXXScopeSpec *SS, clang::CorrectionCandidateCallback &CCC,
      clang::DeclContext *MemberContext, bool EnteringContext,
      const clang::ObjCObjectPointerType *OPT) override {
    // TODO(zarko): This is the primary way we learn about unbound identifiers.
    return clang::TypoCorrection();
  }

  FileTracker *tracker() { return tracker_; }

 private:
  /// The `ActionFactory` orchestrating this multipass run.
  ActionFactory &factory_;

  /// The `FileTracker` keeping track of the file being processed.
  FileTracker *tracker_ = nullptr;
};

void PreprocessorHooks::FileChanged(clang::SourceLocation loc,
                                    clang::PPCallbacks::FileChangeReason reason,
                                    clang::SrcMgr::CharacteristicKind file_type,
                                    clang::FileID prev_fid) {
  if (!enclosing_pass_) {
    return;
  }
  if (reason == clang::PPCallbacks::EnterFile) {
    clang::SourceManager *source_manager =
        &enclosing_pass_->getCompilerInstance().getSourceManager();
    clang::FileID loc_id = source_manager->getFileID(loc);
    if (const clang::FileEntry *file_entry =
            source_manager->getFileEntryForID(loc_id)) {
      if (file_entry->getName() == enclosing_pass_->tracker()->filename()) {
        enclosing_pass_->tracker()->set_file_begin(loc);
        tracked_file_ = file_entry;
      }
    }
  }
}

ActionFactory::ActionFactory(size_t iterations) : iterations_(iterations) {
  for (const auto *file = builtin_headers_create(); file->name; ++file) {
    builtin_headers_.push_back(llvm::MemoryBuffer::getMemBuffer(
        llvm::MemoryBufferRef(file->data, file->name), false));
  }
}

ActionFactory::~ActionFactory() {
  for (auto &tracker : file_trackers_) {
    delete tracker.second;
  }
  file_trackers_.clear();
}

void ActionFactory::RemapFiles(
    llvm::StringRef resource_dir,
    std::vector<std::pair<std::string, llvm::MemoryBuffer *>>
        *remapped_buffers) {
  remapped_buffers->clear();
  for (FileTrackerMap::iterator I = file_trackers_.begin(),
                                E = file_trackers_.end();
       I != E; ++I) {
    FileTracker *tracker = I->second;
    if (llvm::MemoryBuffer *buffer = tracker->memory_buffer()) {
      remapped_buffers->push_back(std::make_pair(tracker->filename(), buffer));
    }
  }
  for (const auto &buffer : builtin_headers_) {
    llvm::SmallString<1024> out_path = resource_dir;
    llvm::sys::path::append(out_path, "include");
    llvm::sys::path::append(out_path, buffer->getBufferIdentifier());
    remapped_buffers->push_back(std::make_pair(out_path.c_str(), buffer.get()));
  }
}

FileTracker *ActionFactory::GetOrCreateTracker(llvm::StringRef filename) {
  FileTrackerMap::iterator i = file_trackers_.find(filename);
  if (i == file_trackers_.end()) {
    FileTracker *new_tracker = new FileTracker(filename);
    file_trackers_[filename] = new_tracker;
    return new_tracker;
  }
  return i->second;
}

void ActionFactory::BeginNextIteration() {
  assert(iterations_ > 0);
  --iterations_;
}

bool ActionFactory::ShouldRunAgain() { return iterations_ > 0; }

bool ActionFactory::runInvocation(clang::CompilerInvocation *invocation,
                                  clang::FileManager *files,
                                  clang::DiagnosticConsumer *diagnostics) {
  // ASTUnit::LoadFromCompilerInvocationAction complains about this too, but
  // we'll leave in our own assert to document the assumption.
  assert(invocation->getFrontendOpts().Inputs.size() == 1);
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_ids(
      new clang::DiagnosticIDs());
  llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diags(
      new clang::DiagnosticsEngine(diag_ids, &invocation->getDiagnosticOpts()));
  if (diagnostics) {
    diags->setClient(diagnostics, false);
  } else {
    diagnostics = new clang::TextDiagnosticPrinter(
        llvm::errs(), &invocation->getDiagnosticOpts());
    diags->setClient(diagnostics, /*ShouldOwnClient*/ true);
  }
  clang::ASTUnit *ast_unit = nullptr;
  // We only consider one full parse on one input file for now, so we only ever
  // need one Action.
  auto action = llvm::make_unique<Action>(*this);
  do {
    BeginNextIteration();
    if (!ast_unit) {
      ast_unit = clang::ASTUnit::LoadFromCompilerInvocationAction(
          invocation, diags, action.get(), ast_unit,
          /*Persistent*/ false, llvm::StringRef(),
          /*OnlyLocalDecls*/ false,
          /*CaptureDiagnostics*/ true,
          /*PrecompilePreamble*/ true,
          /*CacheCodeCompletionResults*/ false,
          /*IncludeBriefCommentsInCodeCompletion*/ false,
          /*UserFilesAreVolatile*/ true,
          /*ErrAST*/ nullptr);
      // The preprocessor hooks must have configured the FileTracker.
      assert(action->tracker() != nullptr);
    } else {
      // ASTUnit::Reparse does the following:
      //   PreprocessorOptions &PPOpts = Invocation->getPreprocessorOpts();
      //   for (const auto &RB : PPOpts.RemappedFileBuffers)
      //     delete RB.second;
      // It then adds back the buffers that were passed to Reparse.
      // Since we don't want our buffers to be deleted, we have to clear out
      // the ones ASTUnit might touch, then pass it a new list.
      invocation->getPreprocessorOpts().RemappedFileBuffers.clear();
      std::vector<std::pair<std::string, llvm::MemoryBuffer *>> buffers;
      RemapFiles(invocation->getHeaderSearchOpts().ResourceDir, &buffers);
      // Reparse doesn't offer any way to run actions, so we're limited here
      // to checking whether our edits were successful (or perhaps to
      // driving new edits only from stored diagnostics). If we need to
      // start from scratch, we'll have to create a new ASTUnit or re-run the
      // invocation entirely. ActionFactory (and FileTracker) are built the
      // way they are to permit them to persist beyond SourceManager/FileID
      // churn.
      ast_unit->Reparse(buffers);
      clang::SourceLocation old_begin = action->tracker()->file_begin();
      clang::FileID old_id = ast_unit->getSourceManager().getFileID(old_begin);
      action->tracker()->BeginPass();
      // Restore the file begin marker, since we won't get any preprocessor
      // events during Reparse. (We can restore other markers if we'd like
      // by computing offsets to this marker.)
      action->tracker()->set_file_begin(
          ast_unit->getSourceManager().getLocForStartOfFile(old_id));
      clang::Rewriter rewriter(ast_unit->getSourceManager(),
                               ast_unit->getLangOpts());
      if (action->tracker()->Rewrite(&rewriter)) {
        action->tracker()->CommitRewrite(old_id, &rewriter);
      } else if (iterations_ == 0) {
        action->tracker()->mark_failed();
      }
    }
  } while (action->tracker()->state() == FileTracker::State::kBusy &&
           ShouldRunAgain());
  if (action->tracker()->state() != FileTracker::State::kFailure) {
    const auto buffer = action->tracker()->backing_store();
    printf("%s", buffer.str().c_str());
  }
  return action->tracker()->state() == FileTracker::State::kSuccess;
}

}  // namespace fyi
}  // namespace kythe
