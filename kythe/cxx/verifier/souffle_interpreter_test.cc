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

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace kythe::verifier {
TEST(SouffleInterpreterTest, SmokeTest) {
  SymbolTable symbols;
  Database db;
  AnchorMap anchors;
  std::vector<GoalGroup> groups;
  std::vector<AssertionParser::Inspection> inspections;
  auto result =
      RunSouffle(symbols, groups, db, anchors, inspections,
                 [](const AssertionParser::Inspection&) { return true; });
  ASSERT_TRUE(result.success);
}
}  // namespace kythe::verifier
