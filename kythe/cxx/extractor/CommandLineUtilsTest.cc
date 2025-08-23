/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::kythe::common::GCCArgsToClangArgs;
using ::kythe::common::HasCxxInputInCommandLineOrArgs;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

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

TEST(GCCArgsToClangArgs, RetainedArgs) {
  EXPECT_THAT(
      GCCArgsToClangArgs({"-Wthread-safety, -Wno-unused-but-set-variable",
                          "-Werror=unused-but-set-parameter",
                          "-Wno-error=unused-local-typedefs"}),
      ElementsAreArray({"-Wthread-safety, -Wno-unused-but-set-variable",
                        "-Werror=unused-but-set-parameter",
                        "-Wno-error=unused-local-typedefs"}));
}

TEST(GCCAargsToClangArgs, DiscardedArgs) {
  EXPECT_THAT(GCCArgsToClangArgs({"-Wmaybe-uninitialized", "-fno-float-store",
                                  "-Werror=thread-unsupported-lock-name",
                                  "-Wno-error=coverage-mismatch"}),
              IsEmpty());
}

// TODO(zarko): Port additional tests.

}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
