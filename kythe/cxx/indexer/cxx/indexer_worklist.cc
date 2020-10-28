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

#include "kythe/cxx/indexer/cxx/indexer_worklist.h"

#include "absl/memory/memory.h"
#include "kythe/cxx/indexer/cxx/IndexerASTHooks.h"

namespace kythe {
namespace {
class IndexerWorklistImpl : public IndexerWorklist {
 public:
  IndexerWorklistImpl(IndexerASTVisitor* indexer) : indexer_(indexer) {}

  void EnqueueJobForImplicitDecl(clang::Decl* decl,
                                 bool set_prune_incomplete_functions,
                                 const std::string& id) override {
    worklist_.emplace_back(absl::make_unique<IndexJob>(
        indexer_->getCurrentJob(), decl, set_prune_incomplete_functions, id));
  }

  void EnqueueJob(std::unique_ptr<IndexJob> job) override {
    worklist_.emplace_back(std::move(job));
  }

  bool DoWork() override {
    std::vector<std::unique_ptr<IndexJob>> jobs = std::move(worklist_);
    for (auto& job : jobs) {
      indexer_->RunJob(std::move(job));
    }
    return !worklist_.empty();
  }

 private:
  /// \brief All queued work.
  std::vector<std::unique_ptr<IndexJob>> worklist_;

  /// \brief The indexer that will execute jobs.
  IndexerASTVisitor* indexer_;
};
}  // anonymous namespace

std::unique_ptr<IndexerWorklist> IndexerWorklist::CreateDefaultWorklist(
    IndexerASTVisitor* indexer) {
  return absl::make_unique<IndexerWorklistImpl>(indexer);
}
}  // namespace kythe
