/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// This file uses the Clang style conventions.

#include "CommandLineUtils.h"
#include "gtest/gtest.h"

namespace {

using ::kythe::common::HasCxxInputInCommandLineOrArgs;
using ::kythe::common::AdjustClangArgsForAnalyze;
using ::kythe::common::AdjustClangArgsForSyntaxOnly;
using ::kythe::common::ClangArgsToGCCArgs;
using ::kythe::common::GCCArgsToClangArgs;
using ::kythe::common::DetermineDriverAction;
using ::kythe::common::DriverAction;
using ::kythe::common::ASSEMBLY;
using ::kythe::common::CXX_COMPILE;
using ::kythe::common::C_COMPILE;
using ::kythe::common::FORTRAN_COMPILE;
using ::kythe::common::GO_COMPILE;
using ::kythe::common::LINK;
using ::kythe::common::UNKNOWN;

TEST(HasCxxInputInCommandLineOrArgs, GoodInputs) {
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a.c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a.c", "b", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.c", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b", "c.c"}));

  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.C", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.c++", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.cc", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.cp", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.cpp", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.cxx", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.i", "c"}));
  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "a", "b.ii", "c"}));

  EXPECT_TRUE(HasCxxInputInCommandLineOrArgs({"-c", "base/timestamp.cc"}));
}

TEST(HasCxxInputInCommandLineOrArgs, BadInputs) {
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"", "", ""}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.o", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.a", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b", "c."}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.ccc", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.ccc.ccc"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.ccc+", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.cppp", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.CC", "c"}));
  EXPECT_FALSE(HasCxxInputInCommandLineOrArgs({"a", "b.xx", "c"}));
  EXPECT_FALSE(
      HasCxxInputInCommandLineOrArgs({"-Wl,@foo", "base/timestamp.cc"}));
  EXPECT_FALSE(
      HasCxxInputInCommandLineOrArgs({"base/timestamp.cc", "-Wl,@foo"}));
}

// TODO(zarko): Port additional tests.

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
