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

#include "absl/strings/string_view.h"
#include "gtest/gtest.h"
#include "kythe/cxx/verifier/assertion_ast.h"
#include "kythe/cxx/verifier/pretty_printer.h"

namespace kythe::verifier {
namespace {

/// \brief a test fixture that makes it easy to manipulate verifier ASTs.
class AstTest : public ::testing::Test {
 protected:
  /// \return an `Identifier` node with value `string`.
  Identifier* Id(const std::string& string) {
    auto symbol = symbol_table_.intern(string);
    return new (&arena_) Identifier(yy::location{}, symbol);
  }

  /// \return a string representation of `node`
  std::string Dump(AstNode* node) {
    StringPrettyPrinter printer;
    node->Dump(symbol_table_, &printer);
    return printer.str();
  }

 private:
  Arena arena_;
  SymbolTable symbol_table_;
};

TEST_F(AstTest, SelfTest) { EXPECT_EQ("test", Dump(Id("test"))); }

}  // anonymous namespace
}  // namespace kythe::verifier
