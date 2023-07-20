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

#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Tooling.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {
using ::clang::ASTUnit;
using ::clang::tooling::buildASTFromCode;
using ::testing::ElementsAre;
using ::testing::Pair;

class ClangRangeFinderTest : public ::testing::Test {
 public:
  clang::ASTUnit& Parse(llvm::StringRef code,
                        llvm::StringRef filename = "input.cc") {
    ast_ = buildASTFromCode(code, filename);
    return *ABSL_DIE_IF_NULL(ast_);
  }

  absl::string_view GetSourceText(clang::SourceRange range) {
    return GetSourceText(clang::CharSourceRange::getCharRange(range));
  }

  absl::string_view GetSourceText(clang::CharSourceRange range) {
    CHECK(range.isValid());
    bool invalid = false;
    auto text = clang::Lexer::getSourceText(range, source_manager(),
                                            lang_options(), &invalid);
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

// Returns a matcher checking for an empty absl::string_view starting at data.
auto EmptyAt(const char* data) {
  return ::testing::AllOf(
      ::testing::Property(&absl::string_view::data, ::testing::Eq(data)),
      ::testing::IsEmpty());
}

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

std::vector<const clang::NamedDecl*> FindAllNamedDecls(clang::ASTUnit& ast) {
  struct NamedDeclCollector : clang::RecursiveASTVisitor<NamedDeclCollector> {
    bool VisitNamedDecl(clang::NamedDecl* decl) {
      decls.push_back(decl);
      return true;
    }

    std::vector<const clang::NamedDecl*> decls;
  } visitor;
  for (auto iter = ast.top_level_begin(); iter != ast.top_level_end(); iter++) {
    visitor.TraverseDecl(*iter);
  }
  return visitor.decls;
}

class NamedDeclTestCase {
 public:
  using DeclFinder = std::function<const clang::NamedDecl*(clang::ASTUnit&)>;
  NamedDeclTestCase(absl::string_view format, absl::string_view name = "entity",
                    DeclFinder find_decl = &FindLastDecl)
      : format_(ABSL_DIE_IF_NULL(absl::ParsedFormat<'s'>::New(format))),
        name_(name),
        find_decl_(std::move(find_decl)) {}
  NamedDeclTestCase(absl::string_view format, DeclFinder find_decl)
      : NamedDeclTestCase(format, "entity", std::move(find_decl)) {}

  std::string SourceText() const { return absl::StrFormat(*format_, name_); }

  const clang::NamedDecl* FindDecl(clang::ASTUnit& ast) const {
    return ABSL_DIE_IF_NULL(find_decl_(ast));
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
      {"#define FUN(T) T ()\nstruct Type { FUN(%s); };", "~Type",
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

TEST_F(ClangRangeFinderTest, NormalizeRangeExpandsZeroWidthRange) {
  ASTUnit& ast = Parse("void func();");
  ASSERT_EQ(
      GetSourceText(range_finder().NormalizeRange(ast.getStartOfMainFileID())),
      "void");
  EXPECT_EQ(GetSourceText(range_finder().NormalizeRange(
                ast.getStartOfMainFileID(), ast.getStartOfMainFileID())),
            "void");
}

// Verifies that references to non-argument macro entities result in a
// zero-width range as the start of the range.
TEST_F(ClangRangeFinderTest, TerribleMacrosAreZeroWidth) {
  // A fairly involved and convoluted macro that, through several layers of
  // expansion, results in a variety of ranges both from the macro body and
  // macro arguments.  We want to ensure that only ranges originating from
  // explicit macro arguments are expanded and all others are zero-width from
  // the front of the macro expansion.
  absl::string_view macro = R"(
#define ASSIGN_OR_RETURN(...)                                            \
  IMPL_GET_VARIADIC_(                                                    \
      (__VA_ARGS__, IMPL_ASSIGN_OR_RETURN_3_, IMPL_ASSIGN_OR_RETURN_2_)) \
  (__VA_ARGS__)
#define IMPL_GET_VARIADIC_HELPER_(_1, _2, _3, NAME, ...) NAME
#define IMPL_GET_VARIADIC_(args) IMPL_GET_VARIADIC_HELPER_ args
#define IMPL_ASSIGN_OR_RETURN_2_(lhs, rexpr) \
  IMPL_ASSIGN_OR_RETURN_3_(lhs, rexpr, _)
#define IMPL_ASSIGN_OR_RETURN_3_(lhs, rexpr, error)                            \
  IMPL_ASSIGN_OR_RETURN_(IMPL_CONCAT_(_status_or_value, __LINE__), lhs, rexpr, \
                         error)
#define IMPL_CONCAT_INNER_(x, y) x##y
#define IMPL_CONCAT_(x, y) IMPL_CONCAT_INNER_(x, y)
#define IMPL_ASSIGN_OR_RETURN_(statusor, lhs, rexpr, error) \
  auto statusor = (rexpr);                                  \
  if (!statusor) {                                          \
    int* _ = nullptr;                                       \
    return (error);                                         \
  }                                                         \
  lhs = *statusor;
)";
  std::string source = absl::StrCat(
      macro, absl::StrJoin({"int* get(int, int);", "int* f() { ", "int a, b;",
                            "ASSIGN_OR_RETURN(int r, get(a, b));",
                            "return nullptr;", "}"},
                           "\n"));
  std::vector<std::pair<std::string, absl::string_view>> ranges;
  for (const auto* decl : FindAllNamedDecls(Parse(source))) {
    if (decl->getName().empty()) continue;
    ranges.push_back({decl->getNameAsString(),
                      GetSourceText(range_finder().RangeForNameOf(decl))});
  }
  llvm::StringRef main_buffer =
      source_manager().getBufferData(source_manager().getMainFileID());
  const char* expansion =
      main_buffer.substr(main_buffer.find("ASSIGN_OR_RETURN(int r")).data();

  EXPECT_THAT(ranges,
              ElementsAre(Pair("get", "get"), Pair("f", "f"), Pair("a", "a"),
                          Pair("b", "b"),
                          Pair("_status_or_value25", EmptyAt(expansion)),
                          Pair("_", EmptyAt(expansion)), Pair("r", "r")));
}

// Verifies that references to non-argument macro entities result in
// the full range of the macro expansion when they are the only token.
TEST_F(ClangRangeFinderTest, SingleTokenMacroRangesUsesFullRange) {
  std::vector<NamedDeclTestCase> decls = {
      {"#define NAMED(a) prefix_##a\n"
       "void %s ();",
       "NAMED(entity)"},
      {"#define NAMED(a) prefix_##a\n"
       "#define entity NAMED(entity)\n"
       "void %s ();"},
  };
  for (const auto& test : decls) {
    ASTUnit& ast = Parse(test.SourceText());
    ClangRangeFinder finder(&source_manager(), &lang_options());

    EXPECT_EQ(GetSourceText(finder.RangeForNameOf(test.FindDecl(ast))),
              test.name());
  }
}

// Verifies that references to non-argument macro entities result in
// the full range of the macro expansion when they are all of the tokens.
TEST_F(ClangRangeFinderTest, MultiTokenMacroRangesUsesFullRange) {
  std::vector<NamedDeclTestCase> decls = {
      {"#define CLASS(a) class prefix_##a { prefix_##a() = default; }\n"
       "CLASS(%s);"},
  };
  for (const auto& test : decls) {
    ASTUnit& ast = Parse(test.SourceText());

    EXPECT_EQ(GetSourceText(range_finder().NormalizeRange(
                  test.FindDecl(ast)->getSourceRange())),
              absl::StrFormat("CLASS(%s)", test.name()))
        << GetSourceText(source_manager().getExpansionRange(
               test.FindDecl(ast)->getSourceRange()));
  }
}
}  // namespace
}  // namespace kythe
