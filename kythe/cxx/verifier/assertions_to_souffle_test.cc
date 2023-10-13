/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/verifier/assertions_to_souffle.h"

#include "absl/log/check.h"
#include "absl/strings/strip.h"
#include "absl/types/span.h"
#include "gtest/gtest.h"
#include "kythe/cxx/verifier/assertion_ast.h"
#include "kythe/cxx/verifier/pretty_printer.h"

namespace kythe::verifier {
namespace {

/// \brief a test fixture that makes it easy to manipulate verifier ASTs.
class AstTest : public ::testing::Test {
 protected:
  AstTest() { p_.set_emit_prelude(false); }

  /// \return an `Identifier` node with value `string`.
  Identifier* Id(const std::string& string) {
    auto symbol = symbol_table_.intern(string);
    return new (&arena_) Identifier(yy::location{}, symbol);
  }

  /// \return an `AstNode` of the form App(`head`, Tuple(`values`))
  AstNode* Pred(AstNode* head, absl::Span<AstNode* const> values) {
    size_t values_count = values.size();
    AstNode** body = (AstNode**)arena_.New(values_count * sizeof(AstNode*));
    size_t vn = 0;
    for (AstNode* v : values) {
      body[vn] = v;
      ++vn;
    }
    AstNode* tuple = new (&arena_) Tuple(yy::location{}, values_count, body);
    return new (&arena_) App(yy::location{}, head, tuple);
  }

  /// \return a `Range` at the given location.
  Range* Range(size_t begin, size_t end, const std::string& path,
               const std::string& root, const std::string& corpus) {
    return new (&arena_) kythe::verifier::Range(
        yy::location{}, begin, end, symbol_table_.intern(path),
        symbol_table_.intern(root), symbol_table_.intern(corpus));
  }

  /// \return a string representation of `node`
  std::string Dump(AstNode* node) {
    StringPrettyPrinter printer;
    node->Dump(symbol_table_, &printer);
    return printer.str();
  }

  /// \return the generated program without boilerplate
  absl::string_view MustGenerateProgram() {
    CHECK(p_.Lower(symbol_table_, {}, {}));
    auto code = p_.code();
    CHECK(absl::ConsumeSuffix(&code, ".\n"));
    return code;
  }

 private:
  Arena arena_;
  SymbolTable symbol_table_;
  SouffleProgram p_;
};

TEST_F(AstTest, SelfTest) {
  EXPECT_EQ("test", Dump(Id("test")));
  EXPECT_EQ("a()", Dump(Pred(Id("a"), {})));
  EXPECT_EQ("a(b, c)", Dump(Pred(Id("a"), {Id("b"), Id("c")})));
  EXPECT_EQ("Range(c,r,p,0,1)", Dump(Range(0, 1, "p", "r", "c")));
}

TEST_F(AstTest, EmptyProgramTest) { EXPECT_EQ("", MustGenerateProgram()); }
}  // anonymous namespace
}  // namespace kythe::verifier
