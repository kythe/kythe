/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_WORKLIST_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_WORKLIST_H_

#include <memory>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "clang/AST/Decl.h"
#include "clang/Sema/Template.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"

namespace kythe {

/// \brief An indexer task and its related state.
struct IndexJob {
  explicit IndexJob(clang::Decl* Decl) : Decl(Decl) {}
  /// \brief Build an IndexJob to visit a file's top-level comment.
  IndexJob(clang::Decl* Decl, clang::FileID Id,
           const GraphObserver::NodeId& FileNode)
      : IsDeclJob(false), Decl(Decl), FileId(Id), FileNode(FileNode) {}
  IndexJob(const IndexJob& ParentJob, clang::Decl* Decl,
           bool SetPruneIncompleteFunctions, std::string ClaimId)
      : Decl(Decl),
        UnderneathImplicitTemplateInstantiation(
            ParentJob.UnderneathImplicitTemplateInstantiation),
        SetPruneIncompleteFunctions(SetPruneIncompleteFunctions),
        RangeContext(ParentJob.RangeContext),
        TypeContext(ParentJob.TypeContext),
        BlameStack(ParentJob.BlameStack),
        ClaimId(std::move(ClaimId)) {}

  /// \brief Is this job meant to index Decl?
  bool IsDeclJob = true;

  /// \brief The `Decl` to index.
  clang::Decl* Decl;

  /// \brief The file ID of the file whose top-level comment should be indexed.
  clang::FileID FileId;

  /// \brief The file node for the file whose top-level comment should be
  /// indexed.
  GraphObserver::NodeId FileNode;

  /// \brief True if we're currently underneath an implicit template
  /// instantiation.
  bool UnderneathImplicitTemplateInstantiation = false;

  /// \brief True if we should only traverse decls that are complete
  /// functions.
  bool PruneIncompleteFunctions = false;

  /// \brief True if we should prune incomplete functions after traversing
  /// `Decl`.
  bool SetPruneIncompleteFunctions = false;

  /// \brief The current context for constructing `GraphObserver::Range`s.
  ///
  /// This is used whenever we enter a context where a section of source
  /// code might have different meaning depending on assignments to
  /// type variables (or, potentially eventually, preprocessor variables).
  /// The `RangeContext` does not need to be extended when the meaning of
  /// a section of code (given the current context) is unambiguous; for example,
  /// the programmer must write down explicit specializations of template
  /// classes (so they have distinct ranges), but the programmer does *not*
  /// write down implicit specializations (so the context must be extended to
  /// give them distinct ranges).
  std::vector<GraphObserver::NodeId> RangeContext;

  /// \brief The current type variable context for the visitor (indexed by
  /// depth).
  std::vector<clang::TemplateParameterList*> TypeContext;

  /// \brief At least 1 NodeId.
  using SomeNodes = llvm::SmallVector<GraphObserver::NodeId, 1>;

  /// \brief A stack of ID groups to use when assigning blame for references
  /// (such as
  /// function calls).
  std::vector<SomeNodes> BlameStack;

  /// \brief A string to represent this job for claiming.
  std::string ClaimId;

  /// \brief A stack of CXXConstructExprs we've already visited.
  std::vector<const clang::CXXConstructExpr*> ConstructorStack;

  /// \brief The current stack of influence sets.
  std::vector<absl::flat_hash_set<const clang::Decl*>> InfluenceSets;
};

class IndexerASTVisitor;

/// \brief Manages indexer tasks.
class IndexerWorklist {
 public:
  /// \brief Create a new default worklist.
  static std::unique_ptr<IndexerWorklist> CreateDefaultWorklist(
      IndexerASTVisitor* visitor);

  /// \brief Enqueue a job to index an implicit declaration.
  /// \param decl the declaration to index.
  /// \param set_prune_incomplete_functions whether to prune incomplete
  /// functions after traversing `decl`.
  /// \param claim_id the ID string to claim for `decl`.
  virtual void EnqueueJobForImplicitDecl(clang::Decl* decl,
                                         bool set_prune_incomplete_functions,
                                         const std::string& claim_id) = 0;

  /// \brief Enqueue a single job.
  virtual void EnqueueJob(std::unique_ptr<IndexJob> job) = 0;

  /// \brief Perform one or more units of work.
  /// \return true if these is more work to be done.
  virtual bool DoWork() = 0;

  virtual ~IndexerWorklist() {}
};

}  // namespace kythe
#endif  // KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
