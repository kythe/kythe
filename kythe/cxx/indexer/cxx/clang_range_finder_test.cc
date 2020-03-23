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
#include "kythe/cxx/indexer/cxx/clang_range_finder.h"

#include <functional>
#include <memory>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Tooling.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {
using ::clang::ASTUnit;
using ::clang::tooling::buildASTFromCode;

class ClangRangeFinderTest : public ::testing::Test {
 public:
  clang::ASTUnit& Parse(llvm::StringRef code,
                        llvm::StringRef filename = "input.cc") {
    ast_ = buildASTFromCode(code, filename);
    return *CHECK_NOTNULL(ast_);
  }

  absl::string_view GetSourceText(clang::SourceRange range) {
    CHECK(range.isValid());
    bool invalid = false;
    auto text =
        clang::Lexer::getSourceText(clang::CharSourceRange::getCharRange(range),
                                    source_manager(), lang_options(), &invalid);
    CHECK(!invalid);
    return absl::string_view(text.data(), text.size());
  }

  const clang::Decl* top_level_back() { return *(ast_->top_level_end() - 1); }

  clang::SourceManager& source_manager() const {
    return ast_->getSourceManager();
  }
  const clang::LangOptions& lang_options() const { return ast_->getLangOpts(); }
  ClangRangeFinder range_finder() const {
    return ClangRangeFinder(&source_manager(), &lang_options());
  }

 private:
  std::unique_ptr<clang::ASTUnit> ast_;
};

const clang::NamedDecl* FindLastDecl(clang::ASTUnit& ast) {
  CHECK(!ast.top_level_empty());
  return clang::dyn_cast<clang::NamedDecl>(*(ast.top_level_end() - 1));
}

template <typename T>
T* FindLastDeclOf(clang::ASTUnit& ast) {
  struct DeclFinder : clang::RecursiveASTVisitor<DeclFinder> {
    bool TraverseDecl(clang::Decl* decl) {
      if (decl == nullptr) {
        return true;
      }
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

class NamedDeclTestCase {
 public:
  using DeclFinder = std::function<const clang::NamedDecl*(clang::ASTUnit&)>;
  NamedDeclTestCase(absl::string_view format, absl::string_view name = "entity",
                    DeclFinder find_decl = &FindLastDecl)
      : format_(CHECK_NOTNULL(absl::ParsedFormat<'s'>::New(format))),
        name_(name),
        find_decl_(std::move(find_decl)) {}
  NamedDeclTestCase(absl::string_view format, DeclFinder find_decl)
      : NamedDeclTestCase(format, "entity", std::move(find_decl)) {}

  std::string SourceText() const { return absl::StrFormat(*format_, name_); }

  const clang::NamedDecl* FindDecl(clang::ASTUnit& ast) const {
    return CHECK_NOTNULL(find_decl_(ast));
  }

  absl::string_view name() const { return name_; }

 private:
  std::shared_ptr<absl::ParsedFormat<'s'>> format_;
  std::string name_;
  DeclFinder find_decl_;
};

TEST_F(ClangRangeFinderTest, CXXNamedDecl) {
  std::vector<NamedDeclTestCase> decls = {
      {"class %s ;"},
      {"struct %s ;"},
      {"union %s ;"},                 // class objects with different keys.
      {"enum %s {};"},                // enumerations.
      {"typedef void ( *%s )();"},    // legacy typedefs.
      {"using %s = int;"},            // using typedef.
      {"namespace %s {}"},            // namespace declarations.
      {"inline namespace %s {}"},     // namespace declarations.
      {"%s {}", "namespace"},         // anonymous namespace.
      {"inline %s {}", "namespace"},  // anonymous inline namespace.
      {"%s {} n;", "struct",
       &FindLastDeclOf<clang::TagDecl>},         // anonymous struct.
      {"int %s ;"},                              // simple var decls.
      {"void %s ();"},                           // trivial function decl.
      {"namespace ns {} namespace %s = ::ns;"},  // namespace alias.
      {"template <typename> struct %s {};"},     // template class.
      {"template <typename> void %s () {}"},     // function template.
      {"enum { %s };",
       &FindLastDeclOf<clang::EnumConstantDecl>},         // enum constants.
      {"struct T; void %s (T&, T&);", "operator    <<"},  // operator overloads.
      {"struct T { %s(); };", "~T",  // standard destructor.
       &FindLastDeclOf<clang::CXXDestructorDecl>},
      {"struct Type { %s(); };", "~Type",  // standard destructor.
       &FindLastDeclOf<clang::CXXDestructorDecl>},
      {"struct Type { %s   (); };", "~ Type",  // standard destructor.
       &FindLastDeclOf<clang::CXXDestructorDecl>},
      {"struct Type { %s   (); };", "compl   Type",  // awkward destructor.
       &FindLastDeclOf<clang::CXXDestructorDecl>},
  };
  for (const auto& test : decls) {
    ASTUnit& ast = Parse(test.SourceText());
    ClangRangeFinder finder(&source_manager(), &lang_options());

    EXPECT_EQ(GetSourceText(finder.RangeForNameOf(test.FindDecl(ast))),
              test.name());
  }
}

TEST_F(ClangRangeFinderTest, CXXMacroDecls) {
  std::vector<NamedDeclTestCase> decls = {
      {"#define CLASS(X) class X\nCLASS(%s) ;"},
      {"#define FUN(T) T ()\nstruct Type { FUN(%sType); };", "~",
       &FindLastDeclOf<clang::CXXDestructorDecl>},
      {"#define  T Type\nstruct Type { %s (); };", "~ T",
       &FindLastDeclOf<clang::CXXDestructorDecl>},

  };
  for (const auto& test : decls) {
    ASTUnit& ast = Parse(test.SourceText());
    ClangRangeFinder finder(&source_manager(), &lang_options());

    EXPECT_EQ(GetSourceText(finder.RangeForNameOf(test.FindDecl(ast))),
              test.name());
  }
}

TEST_F(ClangRangeFinderTest, ObjCNamedDecl) {
  std::vector<NamedDeclTestCase> decls = {
      {"@interface Original\n@end\n@compatibility_alias %s Original;", "Alias",
       &FindLastDecl},
      {"@interface Selector\n  -(int)%s;\n@end\n", "count",
       &FindLastDeclOf<clang::ObjCMethodDecl>},
      {"@interface Selector\n  -(int)%s:(int)a other:(int)b;\n@end\n", "name",
       &FindLastDeclOf<clang::ObjCMethodDecl>},
  };
  for (const auto& test : decls) {
    // Trigger ObjectiveC mode.
    ASTUnit& ast = Parse(test.SourceText(), "input.m");
    ClangRangeFinder finder(&source_manager(), &lang_options());

    EXPECT_EQ(GetSourceText(finder.RangeForNameOf(test.FindDecl(ast))),
              test.name());
  }
}

TEST_F(ClangRangeFinderTest, RangeForTokenAtExpandsSingleToken) {
  ASTUnit& ast = Parse("void func();");
  clang::SourceRange void_range =
      range_finder().RangeForTokenAt(ast.getStartOfMainFileID());
  ASSERT_EQ(GetSourceText(void_range), "void");
  clang::SourceRange func_range =
      range_finder().RangeForTokenAt(void_range.getEnd().getLocWithOffset(1));
  EXPECT_EQ(GetSourceText(func_range), "func");
}

TEST_F(ClangRangeFinderTest, RangeForEntityAtExpandsOperator) {
  for (absl::string_view op : {"operator<<", "operator <<", "operator\n<<"}) {
    ASTUnit& ast = Parse(absl::StrFormat("void %s(int, struct T&);", op));
    clang::SourceRange void_range =
        range_finder().RangeForEntityAt(ast.getStartOfMainFileID());
    ASSERT_EQ(GetSourceText(void_range), "void");
    clang::SourceRange func_range = range_finder().RangeForEntityAt(
        void_range.getEnd().getLocWithOffset(1));
    EXPECT_EQ(GetSourceText(func_range), op);
  }
}

TEST_F(ClangRangeFinderTest, NormalizeRangeExpandsZeroWidthRange) {
  ASTUnit& ast = Parse("void func();");
  ASSERT_EQ(
      GetSourceText(range_finder().NormalizeRange(ast.getStartOfMainFileID())),
      "void");
  EXPECT_EQ(GetSourceText(range_finder().NormalizeRange(
                ast.getStartOfMainFileID(), ast.getStartOfMainFileID())),
            "void");
}
}  // namespace
}  // namespace kythe
