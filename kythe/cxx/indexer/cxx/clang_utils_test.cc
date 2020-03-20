/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
#include "kythe/cxx/indexer/cxx/clang_utils.h"

#include <memory>

#include "absl/strings/string_view.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {
using ::clang::ASTUnit;
using ::clang::tooling::buildASTFromCode;

class ClangUtilsTest : public ::testing::Test {
 public:
  clang::ASTUnit& Parse(llvm::StringRef code) {
    ast_ = buildASTFromCode(code);
    return *CHECK_NOTNULL(ast_);
  }

  absl::string_view GetCharacterData(clang::SourceRange range) {
    CHECK(range.isValid());
    return absl::string_view(
        source_manager().getCharacterData(range.getBegin()),
        source_manager().getFileOffset(range.getEnd()) -
            source_manager().getFileOffset(range.getBegin()));
  }

  const clang::Decl* top_level_back() { return *(ast_->top_level_end() - 1); }

  clang::SourceManager& source_manager() { return ast_->getSourceManager(); }
  const clang::LangOptions& lang_options() const { return ast_->getLangOpts(); }

 private:
  std::unique_ptr<clang::ASTUnit> ast_;
};

template <typename T>
T* FindLastDecl(clang::ASTUnit& ast) {
  struct DeclFinder : clang::RecursiveASTVisitor<DeclFinder> {
    bool TraverseDecl(clang::Decl* decl) {
      if (decl == nullptr) {
        return true;
      }
      decl->dump();
      if (auto found = clang::dyn_cast<T>(decl)) {
        result = found;
      }
      return this->RecursiveASTVisitor::TraverseDecl(decl);
    }

    T* result = nullptr;
  } visitor;
  for (auto iter = ast.top_level_begin(); iter != ast.top_level_end(); iter++) {
    visitor.TraverseDecl(*iter);
  }
  return visitor.result;
}

using SkipWhitespaceTest = ClangUtilsTest;
TEST_F(SkipWhitespaceTest, AdvancesLocationUntilNonWhitespace) {
  ASTUnit& ast = Parse("    \t\n \nclass X;");
  clang::SourceLocation loc = ast.getStartOfMainFileID();
  SkipWhitespace(source_manager(), &loc);
  ASSERT_TRUE(loc.isValid());
  EXPECT_EQ(source_manager().getCharacterData(loc),
            absl::string_view("class X;"));
}

using RangeForASTEntityFromSourceLocationTest = ClangUtilsTest;
TEST_F(RangeForASTEntityFromSourceLocationTest, SimpleNamedDecl) {
  // These should all use the same name for the entity and that entity should be
  // more than a single character. The test is done of the last top-level
  // declaration found in the text, should any setup be needed.
  std::vector<std::string> decls = {
      "class entity ;",
      "struct entity ;",
      "union entity;",              // class objects with different keys.
      "enum entity {};",            // enumerations.
      "typedef void (*entity)();",  // legacy typedefs.
      "using entity = int;",        // using typedef.
      "namespace entity{}",         // namespace declarations.
      "int entity;",                // simple var decls.
      "void entity();",             // trivial function decl.
      "namespace ns {} namespace entity = ::ns;",  // namespace alias.
      "template <typename> struct entity {};",     // template class.
      "template <typename> void entity() {}",      // function template.
  };
  for (const auto& decl : decls) {
    ASTUnit& ast = Parse(decl);
    ASSERT_GE(ast.top_level_size(), 1);
    clang::SourceRange range = RangeForASTEntityFromSourceLocation(
        source_manager(), lang_options(), top_level_back()->getLocation());
    EXPECT_EQ(GetCharacterData(range), "entity");
  }
}

TEST_F(RangeForASTEntityFromSourceLocationTest, EnumConstantName) {
  ASTUnit& ast = Parse(R"(enum { entity };)");
  auto entity = FindLastDecl<clang::EnumConstantDecl>(ast);
  ASSERT_NE(entity, nullptr);
  ASSERT_TRUE(entity->getLocation().isValid());
  clang::SourceRange range = RangeForASTEntityFromSourceLocation(
      source_manager(), lang_options(), entity->getLocation());
  EXPECT_EQ(GetCharacterData(range), "entity");
}

TEST_F(RangeForASTEntityFromSourceLocationTest, OperatorName) {
  Parse(R"(struct T; void operator     <<(T&, T&);)");
  clang::SourceRange range = RangeForASTEntityFromSourceLocation(
      source_manager(), lang_options(), top_level_back()->getLocation());
  EXPECT_EQ(GetCharacterData(range), "operator     <<");
}

}  // namespace
}  // namespace kythe
