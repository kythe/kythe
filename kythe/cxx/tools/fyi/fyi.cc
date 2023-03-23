/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/tools/fyi/fyi.h"

#include <memory>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/ExternalSemaSource.h"
#include "clang/Sema/Sema.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/cxx/common/schema/facts.h"
#include "llvm/Support/Path.h"
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
  llvm::MemoryBuffer* memory_buffer() { return memory_buffer_.get(); }

  llvm::StringRef filename() { return filename_; }

  const llvm::StringRef backing_store() {
    assert(active_buffer_ < 2);
    if (memory_buffer_backing_store_[active_buffer_].empty()) {
      return "";
    }
    auto* store = &memory_buffer_backing_store_[active_buffer_];
    const char* start = store->data();
    // Why the - 1?: MemoryBufferBackingStore ends with a NUL terminator.
    return llvm::StringRef(start, store->size() - 1);
  }

  /// \param Start a new pass involving this `FileTracker`
  void BeginPass() {
    file_begin_ = clang::SourceLocation();
    pass_had_errors_ = false;
  }

  /// Called for each include file we discover is in the file during a major
  /// pass.
  /// Will catch includes that we've added in earlier major passes as well.
  /// \param source_manager the active SourceManager
  /// \param canonical_path the canonical path to the include file
  /// \param uttered_path the path as it appeared in the program
  /// \param is_angled whether angle brackets were used
  /// \param hash_location the source location of the include's \#
  /// \param end_location the source location following the include
  void NextInclude(clang::SourceManager* source_manager,
                   llvm::StringRef canonical_path, llvm::StringRef uttered_path,
                   bool IsAngled, clang::SourceLocation hash_location,
                   clang::SourceLocation end_location) {
    unsigned offset = source_manager->getFileOffset(end_location);
    if (offset > last_include_offset_) {
      last_include_offset_ = offset;
    }
  }

  /// \brief Rewrite the associated source file with our tentative suggestions.
  /// \param rewriter a valid Rewriter.
  /// \return true if changes will be made, false otherwise.
  bool Rewrite(clang::Rewriter* rewriter) {
    if (state_ != State::kBusy) {
      return false;
    }
    if (!pass_had_errors_) {
      state_ = State::kSuccess;
      return false;
    }
    if (!untried_.empty()) {
      auto to_try = *untried_.begin();
      untried_.erase(untried_.begin());
      tried_.insert(to_try);
      rewriter->InsertTextAfter(
          file_begin_.getLocWithOffset(last_include_offset_),
          "\n#include \"" + to_try + "\"\n");
      return true;
    }
    // We have nothing to do, so abort.
    state_ = State::kFailure;
    return false;
  }

  /// \brief Rewrite the old file into a new file, discarding any previously
  /// allocated buffers.
  /// \param file_id the current ID of the file we are rewriting
  /// \param rewriter a valid Rewriter.
  void CommitRewrite(clang::FileID file_id, clang::Rewriter* rewriter) {
    assert(active_buffer_ < 2);
    can_undo_ = true;
    active_buffer_ = 1 - active_buffer_;
    auto* store = &memory_buffer_backing_store_[active_buffer_];
    const clang::RewriteBuffer* buffer = rewriter->getRewriteBufferFor(file_id);
    store->clear();
    llvm::raw_svector_ostream buffer_stream(*store);
    buffer->write(buffer_stream);
    // Required null terminator.
    store->push_back(0);
    const char* start = store->data();
    llvm::StringRef data(start, store->size() - 1);
    memory_buffer_ = llvm::MemoryBuffer::getMemBuffer(data);
  }

  /// \brief Analysis state, maintained across passes.
  enum class State {
    kBusy,     ///< We are trying to repair this file.
    kSuccess,  ///< We have repaired this file (or there is nothing we can do).
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

  /// \brief Decode and possibly take action on a diagnostic received during
  /// a compilation (sub)pass.
  /// \param diagnostic The diagnostic to handle.
  void HandleStoredDiagnostic(clang::StoredDiagnostic& diagnostic) {
    pass_had_errors_ = true;
  }

  /// \brief Add an include to the set of includes to try.
  /// \param include_path The include path to try (as a quoted include).
  void TryInclude(const std::string& include_path) {
    if (!tried_.count(include_path)) {
      untried_.insert(include_path);
    }
  }

  /// \brief Record the initial state of the file before rewriting it.
  /// \param content The content of the file.
  void SetInitialContent(llvm::StringRef content) {
    if (!saw_initial_state_) {
      active_buffer_ = 0;
      memory_buffer_backing_store_[0].clear();
      memory_buffer_backing_store_[0].append(content.begin(), content.end());
      memory_buffer_backing_store_[0].push_back(0);
      memory_buffer_ = llvm::MemoryBuffer::getMemBuffer(backing_store());
      can_undo_ = false;
      saw_initial_state_ = true;
    }
  }

 private:
  friend class Action;

  /// Try to undo the previous change to the backing store.
  bool Undo() {
    assert(active_buffer_ < 2);
    if (can_undo_) {
      active_buffer_ = 1 - active_buffer_;
      memory_buffer_ = llvm::MemoryBuffer::getMemBuffer(backing_store());
      can_undo_ = false;
      return true;
    }
    return false;
  }

  /// The absolute path to the file this FileTracker tracks. Used as a key
  /// to connect between passes.
  std::string filename_;

  /// The location of the beginning of the tracked file. This changes after
  /// each pass.
  clang::SourceLocation file_begin_;

  /// The offset of the last include in the original source file. This will
  /// be used as the insertion point for new include directives.
  unsigned last_include_offset_ = 0;

  /// If this file has been modified, points to a MemoryBuffer containing
  /// the full text of the modified file.
  std::unique_ptr<llvm::MemoryBuffer> memory_buffer_ = nullptr;

  /// Data backing the MemoryBuffer. This is double-buffered, allowing for one
  /// step of undo. `active_buffer_` selects which buffer we should read from.
  llvm::SmallVector<char, 128> memory_buffer_backing_store_[2];

  /// Which backing store is currently active and which is the backup.
  /// Always < 2.
  size_t active_buffer_ = 0;

  /// Can we undo the previous move?
  bool can_undo_ = false;

  /// Have we ever seen the initial state of the file?
  bool saw_initial_state_ = false;

  /// The current of this FileTracker independent of pass.
  State state_ = State::kBusy;

  /// True if the last subpass had (recoverable) errors.
  bool pass_had_errors_ = false;

  /// Includes we've already tried.
  std::set<std::string> tried_;

  /// Includes we have left to try.
  std::set<std::string> untried_;
};

/// \brief During non-reparse passes, PreprocessorHooks listens for events
/// indicating the files being analyzed and their preprocessor directives.
class PreprocessorHooks : public clang::PPCallbacks {
 public:
  /// \param enclosing_pass The `Action` controlling this pass. Not owned.
  explicit PreprocessorHooks(Action* enclosing_pass)
      : enclosing_pass_(enclosing_pass), tracked_file_(nullptr) {}

  /// \copydoc PPCallbacks::FileChanged
  ///
  /// Finds the `FileEntry` and starting `SourceLocation` for each tracked
  /// file on every pass.
  void FileChanged(clang::SourceLocation loc,
                   clang::PPCallbacks::FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind file_type,
                   clang::FileID prev_fid) override;

  /// \copydoc PPCallbacks::InclusionDirective
  ///
  /// When \p SourceFile is the file being tracked by the enclosing pass,
  /// records details about each inclusion directive encountered (such as
  /// the name of the included file, the location of the directive, and so on).
  void InclusionDirective(clang::SourceLocation hash_location,
                          const clang::Token& include_token,
                          llvm::StringRef file_name, bool is_angled,
                          clang::CharSourceRange file_name_range,
                          clang::OptionalFileEntryRef include_file,
                          llvm::StringRef search_path,
                          llvm::StringRef relative_path,
                          const clang::Module* imported,
                          clang::SrcMgr::CharacteristicKind FileType) override;

 private:
  friend class Action;

  /// The current `Action`. Not owned.
  Action* enclosing_pass_;

  /// The `FileEntry` corresponding to the tracker in `enclosing_pass_`.
  /// Not owned.
  const clang::FileEntry* tracked_file_;
};

/// \brief Manages a full parse and any subsequent reparses for a single file.
class Action : public clang::ASTFrontendAction,
               public clang::ExternalSemaSource {
 public:
  explicit Action(ActionFactory& factory) : factory_(factory) {}

  /// \copydoc ASTFrontendAction::BeginInvocation
  bool BeginInvocation(clang::CompilerInstance& CI) override {
    auto* pp_opts = &CI.getPreprocessorOpts();
    pp_opts->RetainRemappedFileBuffers = true;
    pp_opts->AllowPCHWithCompilerErrors = true;
    factory_.RemapFiles(CI.getHeaderSearchOpts().ResourceDir,
                        &pp_opts->RemappedFileBuffers);
    return true;
  }

  /// \copydoc ASTFrontendAction::CreateASTConsumer
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler, llvm::StringRef in_file) override {
    tracker_ = factory_.GetOrCreateTracker(in_file);
    // Don't bother starting a new pass if the tracker is finished.
    if (tracker_->state() == FileTracker::State::kBusy) {
      tracker_->BeginPass();
      compiler.getPreprocessor().addPPCallbacks(
          std::make_unique<PreprocessorHooks>(this));
    }
    return std::make_unique<clang::ASTConsumer>();
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

    clang::CompilerInstance* compiler = &getCompilerInstance();
    assert(!compiler->hasSema() && "CI already has Sema");

    if (hasCodeCompletionSupport() &&
        !compiler->getFrontendOpts().CodeCompletionAt.FileName.empty())
      compiler->createCodeCompletionConsumer();

    clang::CodeCompleteConsumer* completion_consumer = nullptr;
    if (compiler->hasCodeCompletionConsumer())
      completion_consumer = &compiler->getCodeCompletionConsumer();

    compiler->createSema(getTranslationUnitKind(), completion_consumer);
    compiler->getSema().addExternalSource(this);

    clang::ParseAST(compiler->getSema(), compiler->getFrontendOpts().ShowStats,
                    compiler->getFrontendOpts().SkipFunctionBodies);
  }

  /// \brief Copies the tickets from `reply.edge_set` to `request.ticket`.
  /// \return false if no tickets were copied
  template <typename Reply, typename Request>
  bool CopyTicketsFromEdgeSets(const Reply& reply, Request* request) {
    for (const auto& edge_set : reply.edge_sets()) {
      for (const auto& group : edge_set.second.groups()) {
        for (const auto& edge : group.second.edge()) {
          request->add_ticket(edge.target_ticket());
        }
      }
    }
    return request->ticket_size() != 0;
  }

  /// \brief Adds the paths of all /kythe/node/file nodes from `reply.node` to
  /// this Action's `FileTracker`'s include list.
  template <typename Reply>
  void AddFileNodesToTracker(const Reply& reply) {
    for (const auto& parent : reply.nodes()) {
      bool is_file = false;
      for (const auto& fact : parent.second.facts()) {
        if (fact.first == kythe::common::schema::kFactNodeKind) {
          is_file = (fact.second == "/kythe/node/file");
          break;
        }
      }
      if (!is_file) {
        continue;
      }
      auto maybe_uri = URI::FromString(parent.first);
      if (maybe_uri.first) {
        tracker_->TryInclude(maybe_uri.second.v_name().path());
      }
    }
  }

  /// \copydoc ExternalSemaSource::CorrectTypo
  clang::TypoCorrection CorrectTypo(
      const clang::DeclarationNameInfo& typo, int lookup_kind,
      clang::Scope* scope, clang::CXXScopeSpec* scope_spec,
      clang::CorrectionCandidateCallback& callback,
      clang::DeclContext* member_context, bool entering_context,
      const clang::ObjCObjectPointerType* objc_ptr_type) override {
    // Conservatively assume that something went wrong if we had to invoke
    // typo correction.
    tracker_->pass_had_errors_ = true;
    // Look for any name nodes that could help.
    proto::VName name_node;
    name_node.set_signature(typo.getAsString() + "#n");
    name_node.set_language("c++");
    proto::EdgesRequest named_edges_request;
    auto name_uri = URI(name_node).ToString();
    named_edges_request.add_ticket(name_uri);
    // We've found at least one interesting name in the graph. Now we need
    // to figure out which nodes those names are bound to.
    named_edges_request.add_kind(
        absl::StrCat("%", kythe::common::schema::kNamed));
    proto::EdgesReply named_edges_reply;
    std::string error_text;
    if (!factory_.xrefs_->Edges(named_edges_request, &named_edges_reply,
                                &error_text)) {
      absl::FPrintF(stderr, "Xrefs error (named): %s\n", error_text);
      return clang::TypoCorrection();
    }
    // Get information about the places where those nodes were defined.
    proto::EdgesRequest defined_edges_request;
    proto::EdgesReply defined_edges_reply;
    if (!CopyTicketsFromEdgeSets(named_edges_reply, &defined_edges_request)) {
      return clang::TypoCorrection();
    }
    defined_edges_request.add_kind(
        absl::StrCat("%", kythe::common::schema::kDefines));
    if (!factory_.xrefs_->Edges(defined_edges_request, &defined_edges_reply,
                                &error_text)) {
      absl::FPrintF(stderr, "Xrefs error (defines): %s\n", error_text);
      return clang::TypoCorrection();
    }
    // Finally, figure out whether we can make those definition sites visible
    // to the site of the typo by adding an include.
    proto::EdgesRequest childof_request;
    proto::EdgesReply childof_reply;
    if (!CopyTicketsFromEdgeSets(defined_edges_reply, &childof_request)) {
      return clang::TypoCorrection();
    }
    childof_request.add_filter(kythe::common::schema::kFactNodeKind);
    childof_request.add_kind(kythe::common::schema::kChildOf);
    if (!factory_.xrefs_->Edges(childof_request, &childof_reply, &error_text)) {
      absl::FPrintF(stderr, "Xrefs error (childof): %s\n", error_text);
      return clang::TypoCorrection();
    }
    // Add those files to the set of includes to try out.
    AddFileNodesToTracker(childof_reply);
    return clang::TypoCorrection();
  }

  FileTracker* tracker() { return tracker_; }

 private:
  /// The `ActionFactory` orchestrating this multipass run.
  ActionFactory& factory_;

  /// The `FileTracker` keeping track of the file being processed.
  FileTracker* tracker_ = nullptr;
};

void PreprocessorHooks::FileChanged(clang::SourceLocation loc,
                                    clang::PPCallbacks::FileChangeReason reason,
                                    clang::SrcMgr::CharacteristicKind file_type,
                                    clang::FileID prev_fid) {
  if (!enclosing_pass_) {
    return;
  }
  if (reason == clang::PPCallbacks::EnterFile) {
    clang::SourceManager* source_manager =
        &enclosing_pass_->getCompilerInstance().getSourceManager();
    clang::FileID loc_id = source_manager->getFileID(loc);
    if (const clang::FileEntry* file_entry =
            source_manager->getFileEntryForID(loc_id)) {
      if (file_entry->getName() == enclosing_pass_->tracker()->filename()) {
        enclosing_pass_->tracker()->set_file_begin(loc);
        const auto buffer =
            source_manager->getMemoryBufferForFileOrNone(file_entry);
        if (buffer) {
          enclosing_pass_->tracker()->SetInitialContent(buffer->getBuffer());
        }
        tracked_file_ = file_entry;
      }
    }
  }
}

void PreprocessorHooks::InclusionDirective(
    clang::SourceLocation hash_location, const clang::Token& include_token,
    llvm::StringRef file_name, bool is_angled,
    clang::CharSourceRange file_name_range,
    clang::OptionalFileEntryRef include_file, llvm::StringRef search_path,
    llvm::StringRef relative_path, const clang::Module* imported,
    clang::SrcMgr::CharacteristicKind FileType) {
  if (!enclosing_pass_ || !enclosing_pass_->tracker()) {
    return;
  }
  clang::SourceManager* source_manager =
      &enclosing_pass_->getCompilerInstance().getSourceManager();
  auto id_position = source_manager->getDecomposedExpansionLoc(hash_location);
  const auto* source_file =
      source_manager->getFileEntryForID(id_position.first);
  if (source_file == nullptr || !include_file) {
    return;
  }
  if (tracked_file_ == source_file) {
    enclosing_pass_->tracker()->NextInclude(
        source_manager, include_file->getName(), file_name, is_angled,
        hash_location, file_name_range.getEnd());
  }
}

ActionFactory::ActionFactory(std::unique_ptr<XrefsClient> xrefs,
                             size_t iterations)
    : xrefs_(std::move(xrefs)), iterations_(iterations) {
  for (const auto* file = builtin_headers_create(); file->name; ++file) {
    builtin_headers_.push_back(llvm::MemoryBuffer::getMemBufferCopy(
        llvm::StringRef(file->data), file->name));
  }
}

ActionFactory::~ActionFactory() {
  for (auto& tracker : file_trackers_) {
    delete tracker.second;
  }
  file_trackers_.clear();
}

void ActionFactory::RemapFiles(
    llvm::StringRef resource_dir,
    std::vector<std::pair<std::string, llvm::MemoryBuffer*>>*
        remapped_buffers) {
  remapped_buffers->clear();
  for (FileTrackerMap::iterator I = file_trackers_.begin(),
                                E = file_trackers_.end();
       I != E; ++I) {
    FileTracker* tracker = I->second;
    if (llvm::MemoryBuffer* buffer = tracker->memory_buffer()) {
      remapped_buffers->push_back(
          std::make_pair(std::string(tracker->filename()), buffer));
    }
  }
  for (const auto& buffer : builtin_headers_) {
    llvm::SmallString<1024> out_path = resource_dir;
    llvm::sys::path::append(out_path, "include");
    llvm::sys::path::append(out_path, buffer->getBufferIdentifier());
    remapped_buffers->push_back(std::make_pair(out_path.c_str(), buffer.get()));
  }
}

FileTracker* ActionFactory::GetOrCreateTracker(llvm::StringRef filename) {
  FileTrackerMap::iterator i = file_trackers_.find(filename);
  if (i == file_trackers_.end()) {
    FileTracker* new_tracker = new FileTracker(filename);
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

bool ActionFactory::runInvocation(
    std::shared_ptr<clang::CompilerInvocation> invocation,
    clang::FileManager* files,
    std::shared_ptr<clang::PCHContainerOperations> pch_container_ops,
    clang::DiagnosticConsumer* diagnostics) {
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
  clang::ASTUnit* ast_unit = nullptr;
  // We only consider one full parse on one input file for now, so we only ever
  // need one Action.
  llvm::IntrusiveRefCntPtr<Action> action(new Action(*this));
  do {
    BeginNextIteration();
    if (!ast_unit) {
      ast_unit = clang::ASTUnit::LoadFromCompilerInvocationAction(
          invocation, pch_container_ops, diags, action.get(), ast_unit,
          /*Persistent*/ false, llvm::StringRef(),
          /*OnlyLocalDecls*/ false,
          /*CaptureDiagnostics*/ clang::CaptureDiagsKind::All,
          /*PrecompilePreamble*/ true,
          /*CacheCodeCompletionResults*/ false,
          /*UserFilesAreVolatile*/ true,
          /*ErrAST*/ nullptr);
      // The preprocessor hooks must have configured the FileTracker.
      if (action->tracker() == nullptr) {
        absl::FPrintF(stderr, "Error: Never entered input file.\n");
        return false;
      }
    } else {
      // ASTUnit::Reparse does the following:
      //   PreprocessorOptions &PPOpts = Invocation->getPreprocessorOpts();
      //   for (const auto &RB : PPOpts.RemappedFileBuffers)
      //     delete RB.second;
      // It then adds back the buffers that were passed to Reparse.
      // Since we don't want our buffers to be deleted, we have to clear out
      // the ones ASTUnit might touch, then pass it a new list.
      invocation->getPreprocessorOpts().RemappedFileBuffers.clear();
      std::vector<std::pair<std::string, llvm::MemoryBuffer*>> buffers;
      RemapFiles(invocation->getHeaderSearchOpts().ResourceDir, &buffers);
      // Reparse doesn't offer any way to run actions, so we're limited here
      // to checking whether our edits were successful (or perhaps to
      // driving new edits only from stored diagnostics). If we need to
      // start from scratch, we'll have to create a new ASTUnit or re-run the
      // invocation entirely. ActionFactory (and FileTracker) are built the
      // way they are to permit them to persist beyond SourceManager/FileID
      // churn.
      ast_unit->Reparse(pch_container_ops, buffers);
      clang::SourceLocation old_begin = action->tracker()->file_begin();
      clang::FileID old_id = ast_unit->getSourceManager().getFileID(old_begin);
      action->tracker()->BeginPass();
      // Restore the file begin marker, since we won't get any preprocessor
      // events during Reparse. (We can restore other markers if we'd like
      // by computing offsets to this marker.)
      action->tracker()->set_file_begin(
          ast_unit->getSourceManager().getLocForStartOfFile(old_id));
    }
    // Decide whether we can do anything about the diagnostics.
    for (auto d = ast_unit->stored_diag_afterDriver_begin(),
              e = ast_unit->stored_diag_end();
         d != e; ++d) {
      action->tracker()->HandleStoredDiagnostic(*d);
    }
    clang::Rewriter rewriter(ast_unit->getSourceManager(),
                             ast_unit->getLangOpts());
    if (action->tracker()->Rewrite(&rewriter)) {
      // There are actions we should take.
      action->tracker()->CommitRewrite(ast_unit->getSourceManager().getFileID(
                                           action->tracker()->file_begin()),
                                       &rewriter);
    } else if (iterations_ == 0) {
      action->tracker()->mark_failed();
    }
  } while (action->tracker()->state() == FileTracker::State::kBusy &&
           ShouldRunAgain());
  if (action->tracker()->state() != FileTracker::State::kFailure) {
    const auto buffer = action->tracker()->backing_store();
    if (!buffer.empty()) {
      absl::PrintF("%s", buffer.str());
    }
  }
  return action->tracker()->state() == FileTracker::State::kSuccess;
}

}  // namespace fyi
}  // namespace kythe
