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

#include "kythe/cxx/verifier/souffle_interpreter.h"

#include <cstddef>
#include <optional>
#include <string_view>

#include "absl/log/log.h"
#include "gtest/gtest.h"

namespace kythe::verifier {
TEST(SouffleInterpreterTest, SmokeTest) {
  SymbolTable symbols;
  // Intern the symbols required by the prelude.
  symbols.intern("");
  symbols.intern("/kythe/node/kind");
  symbols.intern("anchor");
  symbols.intern("/kythe/loc/start");
  symbols.intern("/kythe/loc/end");
  Database db;
  AnchorMap anchors;
  std::vector<GoalGroup> groups;
  std::vector<Inspection> inspections;
  auto result = RunSouffle(
      symbols, groups, db, anchors, inspections,
      [](const Inspection&, std::optional<std::string_view>) { return true; },
      [](size_t) { return std::string(""); });
  ASSERT_TRUE(result.success);
}
}  // namespace kythe::verifier
