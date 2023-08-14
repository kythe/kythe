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

#include "ImputedConstructorSupport.h"

#include <memory>
#include <optional>
#include <queue>

#include "IndexerASTHooks.h"
#include "absl/strings/str_join.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "re2/re2.h"

namespace kythe {
namespace {

class TargetTypeVisitor : public clang::RecursiveASTVisitor<TargetTypeVisitor> {
 public:
  explicit TargetTypeVisitor(clang::QualType target_type)
      : target_type_(target_type) {}

  const clang::CXXConstructorDecl* FindConstructor(clang::Stmt* stmt) {
    Enqueue(stmt);
    while (!queue_.empty() && matches_.empty()) {
      stmt = queue_.front();
      queue_.pop();
      this->TraverseStmt(stmt);
    }

    return matches_.size() == 1 ? matches_.front() : nullptr;
  }

  bool VisitCallExpr(clang::CallExpr* expr) {
    if (auto* callee = expr->getDirectCallee()) {
      Enqueue(callee->getBody());
    }
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr* expr) {
    if (auto* decl = expr->getConstructor()) {
      if (decl->getThisType()->getPointeeType() == target_type_) {
        matches_.push_back(decl);
      }
      for (auto* init : decl->inits()) {
        Enqueue(init->getInit());
      }
      Enqueue(decl->getBody());
    }
    return true;
  }

 private:
  void Enqueue(clang::Stmt* stmt) {
    if (stmt != nullptr && seen_.insert(stmt).second) {
      queue_.push(stmt);
    }
  }

  const clang::QualType target_type_;

  std::unordered_set<const clang::Stmt*> seen_;
  std::queue<clang::Stmt*> queue_;
  std::vector<const clang::CXXConstructorDecl*> matches_;
};

std::optional<clang::QualType> FindTargetType(
    const clang::FunctionDecl& callee) {
  if (!callee.isFunctionTemplateSpecialization()) return std::nullopt;

  const auto* specialization_info = callee.getTemplateSpecializationInfo();
  if (specialization_info == nullptr) return std::nullopt;

  const auto* const template_args = specialization_info->TemplateArguments;
  if (template_args == nullptr || template_args->size() < 1)
    return std::nullopt;

  // For all of the function templates supported by this function, the
  // first template parameter is the type that's being instantiated.
  // TODO(shahms): Expand this to cover things like `vector<T>::emplace`
  // where the target type is an argument to the class.
  const auto& target_arg = template_args->get(0);
  if (target_arg.getKind() != target_arg.Type) return std::nullopt;

  return target_arg.getAsType()
      .getDesugaredType(callee.getASTContext())
      .getUnqualifiedType();
}

const clang::CXXConstructorDecl* FindConstructor(clang::Stmt* stmt,
                                                 clang::QualType target_type) {
  TargetTypeVisitor visitor(target_type);
  return visitor.FindConstructor(stmt);
}

// Returns the SourceRange covering just the name and template paremeters
// of the direct callee.
clang::SourceRange FindNamedCalleeRange(const clang::Expr* expr) {
  if (expr == nullptr) return clang::SourceRange();
  if (const auto* callee = clang::dyn_cast<const clang::DeclRefExpr>(
          expr->IgnoreParenImpCasts())) {
    return clang::SourceRange(callee->getLocation(),
                              callee->getEndLoc().getLocWithOffset(1));
  }
  return clang::SourceRange();
}

std::function<bool(absl::string_view)> CompilePatterns(
    const std::unordered_set<std::string>& patterns) {
  re2::RE2::Options options;
  options.set_never_capture(true);
  // TODO(shahms): A mechanism for reporting compile errors.
  std::shared_ptr<re2::RE2> regex = std::make_shared<re2::RE2>(
      absl::StrJoin(patterns, "|",
                    [](std::string* out, absl::string_view value) {
                      absl::StrAppend(out, "(", value, ")");
                    }),
      options);
  return [regex](absl::string_view name) {
    return re2::RE2::FullMatch({name.data(), name.size()}, *regex);
  };
}

}  // namespace

ImputedConstructorSupport::ImputedConstructorSupport(
    std::unordered_set<std::string> allowed_constructor_patterns)
    : ImputedConstructorSupport(CompilePatterns(allowed_constructor_patterns)) {
}

ImputedConstructorSupport::ImputedConstructorSupport(
    std::function<bool(absl::string_view)> allow_constructor_name)
    : allow_constructor_name_(std::move(allow_constructor_name)) {}

void ImputedConstructorSupport::InspectCallExpr(
    IndexerASTVisitor& visitor, const clang::CallExpr* call_expr,
    const GraphObserver::Range& range, const GraphObserver::NodeId& callee_id) {
  const auto* callee = call_expr->getDirectCallee();
  if (callee == nullptr) return;

  const auto qual_name = callee->getQualifiedNameAsString();
  if (!allow_constructor_name_(qual_name)) {
    return;
  }

  if (const auto target_type = FindTargetType(*callee)) {
    if (const auto* ctor = FindConstructor(callee->getBody(), *target_type)) {
      auto node_id = visitor.BuildNodeIdForDecl(ctor);
      visitor.RecordCallEdges(range, node_id);
      const auto call_range = FindNamedCalleeRange(call_expr->getCallee());
      if (call_range.isValid()) {
        if (auto range_context =
                visitor.ExplicitRangeInCurrentContext(call_range)) {
          visitor.getGraphObserver().recordDeclUseLocation(
              *range_context, node_id, GraphObserver::Claimability::Unclaimable,
              visitor.IsImplicit(*range_context));
        }
      }
    }
  }
}

}  // namespace kythe
