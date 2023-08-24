/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/cxx/decl_printer.h"

#include <memory>
#include <string_view>
#include <utility>

#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/strip.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"
#include "kythe/cxx/indexer/cxx/indexed_parent_map.h"
#include "kythe/cxx/indexer/cxx/semantic_hash.h"

namespace kythe {
namespace {
using ::clang::ASTUnit;
using ::clang::ast_matchers::classTemplateDecl;
using ::clang::ast_matchers::fieldDecl;
using ::clang::ast_matchers::functionDecl;
using ::clang::ast_matchers::hasName;
using ::clang::ast_matchers::varDecl;
using ::clang::tooling::buildASTFromCode;
using ::testing::ElementsAre;
using ::testing::MatchesRegex;
using ::testing::Pointee;

class ParsedUnit {
 public:
  explicit ParsedUnit(std::unique_ptr<clang::ASTUnit> unit)
      : unit_(ABSL_DIE_IF_NULL(std::move(unit))) {
    observer_.setLangOptions(&unit_->getLangOpts());
  }

  DeclPrinter CreateDeclPrinter() const {
    return DeclPrinter(observer(), hasher(), parent_map());
  }

  clang::ASTContext& ast_context() const { return unit_->getASTContext(); }
  const GraphObserver& observer() const { return observer_; }
  const SemanticHash& hasher() const { return hasher_; }
  const LazyIndexedParentMap& parent_map() const { return parent_map_; }

 private:
  std::unique_ptr<clang::ASTUnit> unit_;

  NullGraphObserver observer_;
  SemanticHash hasher_{[](const auto&) { return ""; }};
  LazyIndexedParentMap parent_map_{
      unit_->getASTContext().getTranslationUnitDecl()};
};

template <typename T = clang::Decl, typename Matcher = void>
const T* FindSingleDeclOrDie(Matcher&& matcher, clang::ASTContext& context) {
  auto bound = clang::ast_matchers::match(
      std::forward<Matcher>(matcher).bind("_decl"), context);
  CHECK(!bound.empty());
  return ABSL_DIE_IF_NULL(bound.front().template getNodeAs<T>("_decl"));
}

MATCHER_P2(HasQualifiedId, printer, matcher, "") {
  auto id = printer.QualifiedId(arg);
  if (ExplainMatchResult(matcher, id, result_listener)) {
    return true;
  }
  *result_listener << "with qualified id " << std::quoted(id);
  return false;
}

TEST(DeclPrinterTest, LinkageChildNames) {
  constexpr std::string_view code = R"c++(
    extern "C" {
    namespace bork {
    extern "C" {
    void Function();
    }
    }  // namespace bork
    }
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));

  const auto* decl = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(), unit.ast_context());

  DeclPrinter printer = unit.CreateDeclPrinter();
  EXPECT_THAT(printer.QualifiedId(*decl), "Function:bork");
}

TEST(DeclPrinterTest, TypeofFunctionParams) {
  constexpr std::string_view code = R"c++(
    void foo(int a, int b);
    extern typeof(foo) bar;
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const auto* bar = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(hasName("bar")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(bar->parameters(),
              ElementsAre(Pointee(HasQualifiedId(printer, "0:bar")),
                          Pointee(HasQualifiedId(printer, "1:bar"))));
}

TEST(DeclPrinterTest, UnnamedFunctionParams) {
  constexpr std::string_view code = R"c++(
    void foo(int, int);
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const auto* bar = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(hasName("foo")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(bar->parameters(),
              ElementsAre(Pointee(HasQualifiedId(printer, "0:foo")),
                          Pointee(HasQualifiedId(printer, "1:foo"))));
}

TEST(DeclPrinterTest, NamedFunctionParams) {
  constexpr std::string_view code = R"c++(
    void foo(int a, int b);
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const auto* bar = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(hasName("foo")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(bar->parameters(),
              ElementsAre(Pointee(HasQualifiedId(printer, "a:foo")),
                          Pointee(HasQualifiedId(printer, "b:foo"))));
}

TEST(DeclPrinterTest, DirectClassTemplate) {
  constexpr std::string_view code = R"c++(
    namespace ns {
    template <typename>
    class Foo {};
    }  // namespace ns
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const auto* decl = FindSingleDeclOrDie<clang::ClassTemplateDecl>(
      classTemplateDecl(hasName("Foo")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(decl, Pointee(HasQualifiedId(printer, "Foo:ns")));
}

TEST(DeclPrinterTest, ClassTemplateAncestor) {
  constexpr std::string_view code = R"c++(
    namespace ns {
    template <typename>
    class Foo {
      void member();
    };
    }  // namespace ns
  )c++";
  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const auto* decl = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(hasName("member")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(decl, Pointee(HasQualifiedId(printer, "member:Foo:ns")));
}

TEST(DeclPrinterTest, LambdaAncestor) {
  constexpr std::string_view code = R"c++(
    auto a = [] { int foo; };
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  const clang::VarDecl* foo = FindSingleDeclOrDie<clang::VarDecl>(
      varDecl(hasName("foo")), unit.ast_context());
  DeclPrinter printer = unit.CreateDeclPrinter();

  EXPECT_THAT(foo,
              Pointee(HasQualifiedId(
                  printer, MatchesRegex(R"(foo:0:0:operator\(\):[^:]+:0:a)"))));
}

TEST(DeclPrinterTest, AnonymousStruct) {
  constexpr std::string_view code = R"c++(
    struct Outer {
      struct {
        char foo;
      } a;
      struct {
        int bar;
      } b;
    };
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  DeclPrinter printer = unit.CreateDeclPrinter();
  const auto* foo = FindSingleDeclOrDie<clang::FieldDecl>(
      fieldDecl(hasName("foo")), unit.ast_context());
  const auto* bar = FindSingleDeclOrDie<clang::FieldDecl>(
      fieldDecl(hasName("bar")), unit.ast_context());

  EXPECT_THAT(foo->getParent(),
              Pointee(HasQualifiedId(printer, MatchesRegex("[^:]+:Outer"))));
  EXPECT_THAT(bar->getParent(),
              Pointee(HasQualifiedId(printer, MatchesRegex("[^:]+:Outer"))));
  EXPECT_NE(printer.QualifiedId(*foo->getParent()),
            printer.QualifiedId(*bar->getParent()));
}

// Disabled because the compiler-generated class for lambdas
// presently all have the same hash.
TEST(DeclPrinterTest, DISABLED_DistinctLambdaTypes) {
  constexpr std::string_view code = R"c++(
    void wrapper() {
      [] { int foo; }();
      [] { int bar; }();
    }
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  DeclPrinter printer = unit.CreateDeclPrinter();
  const auto* foo = FindSingleDeclOrDie<clang::VarDecl>(varDecl(hasName("foo")),
                                                        unit.ast_context());
  const auto* bar = FindSingleDeclOrDie<clang::VarDecl>(varDecl(hasName("bar")),
                                                        unit.ast_context());

  // Lambda class types should be unique, but it's a bit tricky to uniquely
  // identify them in a way which doesn't impact the name, so strip off the
  // identifier for the inner variable instead.
  EXPECT_EQ(absl::StripPrefix(printer.QualifiedId(*foo), "foo:"),
            absl::StripPrefix(printer.QualifiedId(*bar), "bar:"));
}

TEST(DeclPrinterTest, UnnamedDecl) {
  using ::clang::ast_matchers::staticAssertDecl;
  constexpr std::string_view code = R"c++(
    template <bool valid>
    void wrapper() {
      static_assert(valid, "");
    }
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  DeclPrinter printer = unit.CreateDeclPrinter();
  const auto* decl = FindSingleDeclOrDie<clang::StaticAssertDecl>(
      staticAssertDecl(), unit.ast_context());

  // This Id is longer than expected due to the structure being:
  // [ignored] FunctionTemplateDecl
  //   FunctionDecl
  //     CompoundDecl
  //       StmtDecl
  //         StaticAssertDecl
  EXPECT_THAT(decl, Pointee(HasQualifiedId(printer, "0:0:0:wrapper")));
}

TEST(DeclPrinterTest, AnonymousNamespace) {
  constexpr std::string_view code = R"c++(
    namespace {
    void foo();
    }
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  DeclPrinter printer = unit.CreateDeclPrinter();
  const auto* decl = FindSingleDeclOrDie<clang::FunctionDecl>(
      functionDecl(hasName("foo")), unit.ast_context());

  // The observer is responsible for the "isMainSourceFile" check and
  // adding the corresponding chunk to the stream, so will be empty here.
  EXPECT_THAT(decl, Pointee(HasQualifiedId(printer, "foo:@#anon")));
}

TEST(DeclPrinterTest, UnresolvedUsingValueDecl) {
  using ::clang::ast_matchers::unresolvedUsingValueDecl;
  constexpr std::string_view code = R"c++(
    template <typename>
    struct Base {};
    template <typename T>
    struct Child : Base<T> {
      using Base<T>::Base;
    };
  )c++";

  ParsedUnit unit(buildASTFromCode(code, "input.cc"));
  DeclPrinter printer = unit.CreateDeclPrinter();
  const auto* decl = FindSingleDeclOrDie<clang::UnresolvedUsingValueDecl>(
      unresolvedUsingValueDecl(), unit.ast_context());

  EXPECT_THAT(decl,
              Pointee(HasQualifiedId(printer, MatchesRegex(R"([0-9]:Child)"))));
}

}  // namespace
}  // namespace kythe
