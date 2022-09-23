/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include "objc_bazel_support.h"

#include "absl/log/initialize.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {

TEST(ObjcExtractorBazelMain, TestSanitizeSimple) {
  std::string v = SanitizeArgument("V A L");
  EXPECT_EQ("'V A L'", v);
}

TEST(ObjcExtractorBazelMain, TestSanitizeWithQuote) {
  std::string v = SanitizeArgument(R"(V "A" L)");
  EXPECT_EQ(R"('V "A" L')", v);
}

TEST(ObjcExtractorBazelMain, TestSanitizeWithQuote2) {
  std::string v = SanitizeArgument("V 'A' L");
  EXPECT_EQ(R"("V 'A' L")", v);
}

TEST(ObjcExtractorBazelMain, TestSanitizeWithAllSpecials) {
  std::string v = SanitizeArgument(R"(V \ 'A' $ ` " L)");
  EXPECT_EQ(R"("V \\ 'A' \$ \` \" L")", v);
}

TEST(ObjcExtractorBazelMain, TestSanitizeWithAllDoubleSpecials) {
  std::string v = SanitizeArgument(R"(V \\ 'A' $$ `` " L ")");
  EXPECT_EQ(R"("V \\\\ 'A' \$\$ \`\` \" L \"")", v);
}

TEST(ObjcExtractorBazelMain, TestExtractOneEnvVar) {
  std::vector<std::string> args;
  blaze::SpawnInfo si;
  auto* e = si.add_variable();
  e->set_name("VONE");
  e->set_value("VAL");

  std::string v = BuildEnvVarCommandPrefix(si.variable());

  EXPECT_EQ("VONE='VAL' ", v);
}

TEST(ObjcExtractorBazelMain, TestExtractManyEnvVars) {
  std::vector<std::string> args;
  blaze::SpawnInfo si;
  auto* e = si.add_variable();
  e->set_name("VONE");
  e->set_value("VAL");
  e = si.add_variable();
  e->set_name("VTWO");
  e->set_value("");
  e = si.add_variable();
  e->set_name("VTHREE");
  e->set_value("space space");
  e = si.add_variable();
  e->set_name("VFOUR");
  // This is probably invalid input to the spawn info, but we should be able to
  // handle it without doing something unpredictable.
  e->set_value("space\\ space");
  e = si.add_variable();
  e->set_name("VFIVE");
  e->set_value("B 'A' G$");

  std::string v = BuildEnvVarCommandPrefix(si.variable());

  EXPECT_EQ(
      "VONE='VAL' VTWO='' VTHREE='space space' VFOUR='space\\ space' VFIVE=\"B "
      "'A' G\\$\" ",
      v);
}

TEST(ObjcExtractorBazelMain, TestExtractIgnoreInvalidVarNames) {
  std::vector<std::string> args;
  blaze::SpawnInfo si;
  auto* e = si.add_variable();
  e->set_name("VONE");
  e->set_value("VAL");
  e = si.add_variable();
  e->set_name("V TWO");
  e->set_value("");
  e = si.add_variable();
  e->set_name("VT-HREE");
  e->set_value("space space");
  e = si.add_variable();
  e->set_name(" VFOUR");
  e->set_value("value");
  e = si.add_variable();
  e->set_name("VFIVE");
  e->set_value("T");

  std::string v = BuildEnvVarCommandPrefix(si.variable());

  EXPECT_EQ("VONE='VAL' VFIVE='T' ", v);
}

TEST(ObjcExtractorBazelMain, TestFixArgs) {
  std::vector<std::string> args;
  blaze::SpawnInfo si;
  si.add_argument("__BAZEL_XCODE_DEVELOPER_DIR__/foo");
  si.add_argument("__BAZEL_XCODE_SDKROOT__/bar");
  // This shouldn't happen but we don't have assurance that it won't.
  si.add_argument("__BAZEL_XCODE_SDKROOT__/__BAZEL_XCODE_SDKROOT__/bar");
  // This shouldn't happen but we don't have assurance that it won't.
  si.add_argument("__BAZEL_XCODE_SDKROOT__/__BAZEL_XCODE_DEVELOPER_DIR__/bar");
  si.add_argument("__BAZEL_XCODE_SDKROOT/bar");
  si.add_argument("/baz/bar");
  FillWithFixedArgs(args, si, "/usr/devdir", "/usr/sdkroot");

  // This is verbose so we don't have to pull in a new dep for gmock.
  EXPECT_EQ("/usr/devdir/foo", args[0]);
  EXPECT_EQ("/usr/sdkroot/bar", args[1]);
  EXPECT_EQ("/usr/sdkroot//usr/sdkroot/bar", args[2]);
  EXPECT_EQ("/usr/sdkroot//usr/devdir/bar", args[3]);
  EXPECT_EQ("__BAZEL_XCODE_SDKROOT/bar", args[4]);
  EXPECT_EQ("/baz/bar", args[5]);
}

// Simple test for running a command that should work on (at least) macOS and
// Linux.
TEST(ObjcExtractorBazelMain, TestRunScript) {
  EXPECT_EQ("hi", RunScript("echo '  hi  '"));
}

TEST(ObjcExtractorBazelMain, TestRunScriptThatFails) {
  EXPECT_EQ("", RunScript("ls --fail"));
}

// This test passes on linux (2016-09-09). It seems too dangerous to call out
// to an unknown binary, so this test is commented out so it does not run
// automatically.
// TEST(ObjcExtractorBazelMain, TestRunScriptThatDoesNotExist) {
//  EXPECT_EQ("", RunScript("/usr/bin/xcrun"));
//}

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
