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

#include <regex>

#include "verifier.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace kythe {
namespace verifier {
namespace {

TEST(VerifierUnitTest, StringPrettyPrinter) {
  StringPrettyPrinter c_string;
  c_string.Print("c_string");
  EXPECT_EQ("c_string", c_string.str());
  StringPrettyPrinter std_string;
  std_string.Print(std::string("std_string"));
  EXPECT_EQ("std_string", std_string.str());
  StringPrettyPrinter zero_ptrvoid;
  void *zero = nullptr;
  zero_ptrvoid.Print(zero);
  EXPECT_EQ("0", zero_ptrvoid.str());
  void *one = reinterpret_cast<void *>(1);
  StringPrettyPrinter one_ptrvoid;
  one_ptrvoid.Print(one);
  // Some implementations might pad stringified void*s.
  std::string one_str = one_ptrvoid.str();
  std::regex ptrish_regex("0x0*1");
  ASSERT_TRUE(std::regex_match(one_str, ptrish_regex));
}

TEST(VerifierUnitTest, QuotingPrettyPrinter) {
  StringPrettyPrinter c_string;
  QuoteEscapingPrettyPrinter c_string_quoting(c_string);
  c_string_quoting.Print(R"("hello")");
  EXPECT_EQ(R"(\"hello\")", c_string.str());
  StringPrettyPrinter std_string;
  QuoteEscapingPrettyPrinter std_string_quoting(std_string);
  std_string_quoting.Print(std::string(R"(<"'
)"));
  EXPECT_EQ(R"(<\"\'\n)", std_string.str());
  StringPrettyPrinter zero_ptrvoid;
  QuoteEscapingPrettyPrinter zero_ptrvoid_quoting(zero_ptrvoid);
  void *zero = nullptr;
  zero_ptrvoid_quoting.Print(zero);
  EXPECT_EQ("0", zero_ptrvoid.str());
}

TEST(VerifierUnitTest, HtmlPrettyPrinter) {
  StringPrettyPrinter c_string;
  HtmlEscapingPrettyPrinter c_string_html(c_string);
  c_string_html.Print(R"("hello")");
  EXPECT_EQ(R"(&quot;hello&quot;)", c_string.str());
  StringPrettyPrinter std_string;
  HtmlEscapingPrettyPrinter std_string_html(std_string);
  std_string_html.Print(std::string(R"(x<>&"ml)"));
  EXPECT_EQ(R"(x&lt;&gt;&amp;&quot;ml)", std_string.str());
  StringPrettyPrinter zero_ptrvoid;
  HtmlEscapingPrettyPrinter zero_ptrvoid_html(zero_ptrvoid);
  void *zero = nullptr;
  zero_ptrvoid_html.Print(zero);
  EXPECT_EQ("0", zero_ptrvoid.str());
}

TEST(VerifierUnitTest, UnescapeStringLiterals) {
  std::string tmp = "tmp";
  EXPECT_TRUE(AssertionParser::Unescape(R"("")", &tmp));
  EXPECT_EQ("", tmp);
  EXPECT_FALSE(AssertionParser::Unescape("", &tmp));
  EXPECT_TRUE(AssertionParser::Unescape(R"("foo")", &tmp));
  EXPECT_EQ("foo", tmp);
  EXPECT_TRUE(AssertionParser::Unescape(R"("\"foo\"")", &tmp));
  EXPECT_EQ("\"foo\"", tmp);
  EXPECT_FALSE(AssertionParser::Unescape(R"("\foo")", &tmp));
  EXPECT_FALSE(AssertionParser::Unescape(R"("foo\")", &tmp));
  EXPECT_TRUE(AssertionParser::Unescape(R"("\\")", &tmp));
  EXPECT_EQ("\\", tmp);
}

TEST(VerifierUnitTest, TrivialHappyCase) {
  Verifier v;
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EmptyProtoIsNotWellFormed) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
})"));
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EmptyVnameIsNotWellFormed) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, NoRulesIsOk) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, DuplicateFactsNotWellFormed) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
}
entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, DuplicateFactsNotWellFormedEvenIfYouSeparateThem) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
}
entries {
  source { root: "2" }
  fact_name: "testname"
  fact_value: "testvalue"
}
entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, DuplicateEdgesAreUseless) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  edge_kind: "somekind"
  target { root: "2" }
  fact_name: "/"
  fact_value: ""
}
entries {
  source { root: "1" }
  edge_kind: "somekind"
  target { root: "2" }
  fact_name: "/"
  fact_value: ""
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EdgesCanSupplyMultipleOrdinals) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  edge_kind: "somekind"
  target { root: "2" }
  fact_name: "/kythe/ordinal"
  fact_value: "42"
}
entries {
  source { root: "1" }
  edge_kind: "somekind"
  target { root: "2" }
  fact_name: "/kythe/ordinal"
  fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EdgesCanSupplyMultipleDotOrdinals) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  edge_kind: "somekind.42"
  target { root: "2" }
  fact_name: "/"
}
entries {
  source { root: "1" }
  edge_kind: "somekind.43"
  target { root: "2" }
  fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ConflictingFactsNotWellFormed) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
}
entries {
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue2"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, OnlyTargetIsWrong) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  edge_kind: "somekind"
  target { root: "2" }
  fact_name: "/kythe/ordinal"
  fact_value: "42"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, OnlyTargetIsWrongDotOrdinal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
  edge_kind: "somekind.42"
  target { root: "2" }
  fact_name: "/"
})"));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, MissingAnchorTextFails) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
}

TEST(VerifierUnitTest, AmbiguousAnchorTextFails) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
# text text
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
}

TEST(VerifierUnitTest, GenerateAnchorEvarFailsOnEmptyDB) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @text defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, OffsetsVersusRuleBlocks) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @text defines SomeNode
#- @+2text defines SomeNode
#- @+1text defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ZeroRelativeLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+0text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, NoMatchingInsideGoalComments) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+1text defines SomeNode
#- @text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, OutOfBoundsRelativeLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+2text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, EndOfFileAbsoluteLineReferencesWork) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @:3text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, OutOfBoundsAbsoluteLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:4text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, ZeroAbsoluteLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:0text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, SameAbsoluteLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:1text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, HistoricalAbsoluteLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#
#- @:1text defines SomeNode
# text
)"));
}

TEST(VerifierUnitTest, ParseLiteralString) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text" defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ParseLiteralStringWithSpace) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text txet" defines SomeNode
# text txet
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ParseLiteralStringWithEscape) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text \"txet\" ettx" defines SomeNode
# text "txet" ettx
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateStartOffsetEVar) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- ANode.loc/start @^text
##text (line 3 column 2 offset 38-42)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "38"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "42"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateStartOffsetEVarRelativeLine) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- ANode.loc/start @^+22text
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "387"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "391"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
##text (line 24 column 2 offset 387-391))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateEndOffsetEVarAbsoluteLine) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#-   ANode.loc/end @$:24text
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "387"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "391"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
##text (line 24 column 2 offset 387-391))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateEndOffsetEVar) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#-   ANode.loc/end @$text
##text (line 3 column 2 offset 38-42)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "38"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "42"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvar) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
##text (line 3 column 2 offset 38-42)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "38"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "42"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

constexpr char kMatchAnchorSubgraph[] = R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "40"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "44"
}
entries {
source { root:"3" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/start"
fact_value: "45"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/end"
fact_value: "49"
}
entries {
source { root:"4" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"4" }
fact_name: "/kythe/loc/start"
fact_value: "50"
}
entries {
source { root:"4" }
fact_name: "/kythe/loc/end"
fact_value: "54"
})";

TEST(VerifierUnitTest, GenerateAnchorEvarMatchNumber0) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#0text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarMatchNumber1) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#1text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarMatchNumber2) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#2text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"4" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarMatchNumber3) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @#3text defines SomeNode
##text text text(40-44, 45-49, 50-54)
})"));
}

TEST(VerifierUnitTest, GenerateAnchorEvarMatchNumberNegative1) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @#-1text defines SomeNode
##text text text(40-44, 45-49, 50-54)
})"));
}

TEST(VerifierUnitTest, GenerateAnchorEvarAbsoluteLine) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @:24text defines SomeNode
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "387"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "391"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
##text (line 24 column 2 offset 387-391))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarRelativeLine) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @+22text defines SomeNode
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "387"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "391"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
##text (line 24 column 2 offset 387-391))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarAtEndOfFile) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "384"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "388"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
#- @text defines SomeNode
##text (line 22 column 2 offset 384-388))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarAtEndOfFileWithSpaces) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "386"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "390"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
  #- @text defines SomeNode
##text (line 22 column 2 offset 386-390))"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarAtEndOfFileWithTrailingRule) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "384"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "388"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
#- @text defines SomeNode
##text (line 22 column 2 offset 384-388))
#- SomeAnchor defines SomeNode)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarAcrossMultipleGoalLines) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text defines
#-
#-   SomeNode
##text (line 5 column 2 offset 46-50)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "46"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "50"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarAcrossMultipleGoalLinesWithSpaces) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text defines
  #-
 #-   SomeNode
  ##text (line 5 column 4 offset 51-55)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "51"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "55"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarWithBlankLines) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "384"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "388"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
#- @texx defines SomeNode
##texx (line 22 column 2 offset 384-388))

#-  SomeAnchor defines SomeNode)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateAnchorEvarWithWhitespaceLines) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "384"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "388"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
#- @texx defines SomeNode
##texx (line 22 column 2 offset 384-388))
  	 
#-  SomeAnchor defines SomeNode)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateTwoSeparatedAnchorEvars) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
##text (line 3 column 2 offset 38-42)
#- @more defines OtherNode
#more (line 5 column 1 offset 102-106
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "38"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "42"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
source { root:"3" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/start"
fact_value: "102"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/end"
fact_value: "106"
}
entries {
source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"4" }
fact_name: "/"
fact_value: ""
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateTwoSeparatedAnchorEvarsWithSpaces) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @" text" defines SomeNode
 ## text (line 3 column 3 offset 42-47)
#- @" more	" defines OtherNode
# more	 (line 5 column 1 offset 111-117
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "42"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "47"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
source { root:"3" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/start"
fact_value: "111"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/end"
fact_value: "117"
}
entries {
source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"4" }
fact_name: "/"
fact_value: ""
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GenerateTwoAnchorEvars) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @more defines SomeNode
#- @text defines OtherNode
##text (line 4 column 2 offset 65-69) more (line 4 column 38 offset 101-105)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "65"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "69"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
source { root:"3" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/start"
fact_value: "101"
}
entries {
source { root:"3" }
fact_name: "/kythe/loc/end"
fact_value: "105"
}
entries {
source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"4" }
fact_name: "/"
fact_value: ""
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ContentFactPasses) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EVarsUnsetAfterNegatedBlock) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{vname(_,_,Root?="3",_,_) defines SomeNode}
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  size_t call_count = 0;
  bool evar_unset = false;
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals([&call_count, &evar_unset](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    if (inspection.label == "Root" && !inspection.evar->current()) {
      evar_unset = true;
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(evar_unset);
}

TEST(VerifierUnitTest, NegatedBlocksDontLeak) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{vname(_,_,Root="3",_,_) defines SomeNode}
#- !{vname(_,_,Root?,_,_) notanedge OtherNode}
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  size_t call_count = 0;
  bool evar_unset = false;
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals([&call_count, &evar_unset](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    if (inspection.label == "Root" && !inspection.evar->current()) {
      evar_unset = true;
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(evar_unset);
}

TEST(VerifierUnitTest, UngroupedEVarsAvailableInGroupedContexts) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(_,_,Root="3",_,_) defines SomeNode
#- !{vname(_,_,Root?,_,_) defines SomeNode}
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"4" }
fact_name: "/"
fact_value: ""
})"));
  size_t call_count = 0;
  bool evar_set = false;
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals([&call_count, &evar_set](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    if (inspection.label == "Root" && inspection.evar->current()) {
      if (Identifier *identifier = inspection.evar->current()->AsIdentifier()) {
        if (cxt->symbol_table()->text(identifier->symbol()) == "3") {
          evar_set = true;
        }
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(evar_set);
}

TEST(VerifierUnitTest, UngroupedEVarsAvailableInGroupedContextsReordered) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{vname(_,_,Root?,_,_) defines SomeNode}
#- vname(_,_,Root="3",_,_) defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"4" }
fact_name: "/"
fact_value: ""
})"));
  size_t call_count = 0;
  bool evar_set = false;
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals([&call_count, &evar_set](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    if (inspection.label == "Root" && inspection.evar->current()) {
      if (Identifier *identifier = inspection.evar->current()->AsIdentifier()) {
        if (cxt->symbol_table()->text(identifier->symbol()) == "3") {
          evar_set = true;
        }
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(evar_set);
}

TEST(VerifierUnitTest, EVarsReportedAfterFailedNegatedBlock) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{vname(_,_,Root?="1",_,_) defines SomeNode}
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  size_t call_count = 0;
  bool evar_set = false;
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals([&call_count, &evar_set](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    if (inspection.label == "Root" && inspection.evar->current()) {
      if (Identifier *identifier = inspection.evar->current()->AsIdentifier()) {
        if (cxt->symbol_table()->text(identifier->symbol()) == "1") {
          evar_set = true;
        }
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(evar_set);
}

TEST(VerifierUnitTest, GroupFactFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- { SomeNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

// Slightly brittle in that it depends on the order we try facts.
TEST(VerifierUnitTest, FailWithCutInGroups) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- { SomeNode.content SomeValue }
#- { SomeNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

// Slightly brittle in that it depends on the order we try facts.
TEST(VerifierUnitTest, FailWithCut) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content SomeValue
#- { SomeNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, PassWithoutCut) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content SomeValue
#- SomeNode.content 43
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GroupFactPasses) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- { SomeNode.content 42 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, CompoundGroups) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42
#- !{ OtherNode.content 43 
#-    AnotherNode.content 44 }
#- !{ LastNode.content 45 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ConjunctionInsideNegatedGroupPassFail) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 42 
#-    OtherNode.content 44 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ConjunctionInsideNegatedGroupPassPass) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 42 
#-    OtherNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
}
entries {
source { root:"2" }
fact_name: "/kythe/content"
fact_value: "43"
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, AntiContentFactFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 42 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, AntiContentFactPasses) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, SpacesAreOkay) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
   	 #- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ContentFactFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, SpacesDontDisableRules) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
   	 #- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, DefinesEdgePasses) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgePasses) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param"
target { root:"2" }
fact_name: "/kythe/ordinal"
fact_value: "1"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgePassesDotOrdinal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param.1"
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeBadDotOrdinalNoNumber) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param."
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeBadDotOrdinalOnlyDot) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "."
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeBadDotOrdinalNoEdge) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: ".42"
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeFailsOnWrongOrdinal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param"
target { root:"2" }
fact_name: "/kythe/ordinal"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeFailsOnWrongDotOrdinal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.1 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param.42"
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeFailsOnMissingOrdinalInGoal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param"
target { root:"2" }
fact_name: "/kythe/ordinal"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeFailsOnMissingDotOrdinalInGoal) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param.42"
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IsParamEdgeFailsOnMissingOrdinalInFact) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeParam is_param.42 SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EvarsShareANamespace) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor defines SomeNode
#- SomeNode defines SomeAnchor
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, DefinesEdgePassesSymmetry) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
#- SomeNode defined_by SomeAnchor
source { root:"2" }
edge_kind: "/kythe/edge/defined_by"
target { root:"1" }
fact_name: "/"
fact_value: ""
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EvarsStillShareANamespace) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
}
entries {
#- SomeAnchor defined_by SomeNode
source { root:"2" }
edge_kind: "/kythe/edge/defined_by"
target { root:"1" }
fact_name: "/"
fact_value: ""
}
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, InspectionCalledFailure) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals([](
      Verifier *cxt, const AssertionParser::Inspection &) { return false; }));
}

TEST(VerifierUnitTest, EvarsAreSharedAcrossInputFiles) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- SomeAnchor? defines _
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  EVar *seen_evar = nullptr;
  int seen_count = 0;
  ASSERT_TRUE(v.VerifyAllGoals([&seen_evar, &seen_count](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (inspection.label == "SomeAnchor") {
      ++seen_count;
      if (seen_evar == nullptr) {
        seen_evar = inspection.evar;
      } else if (seen_evar != inspection.evar) {
        return false;
      }
    }
    return true;
  }));
  ASSERT_EQ(2, seen_count);
  ASSERT_NE(nullptr, seen_evar);
}

TEST(VerifierUnitTest, InspectionCalledSuccess) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals(
      [](Verifier *cxt, const AssertionParser::Inspection &) { return true; }));
}

TEST(VerifierUnitTest, InspectionHappensMoreThanOnceAndThatsOk) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode
#- SomeAnchor? defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  size_t inspect_count = 0;
  ASSERT_TRUE(v.VerifyAllGoals(
      [&inspect_count](Verifier *cxt, const AssertionParser::Inspection &) {
        ++inspect_count;
        return true;
      }));
  ASSERT_EQ(2, inspect_count);
}

TEST(VerifierUnitTest, InspectionCalledCorrectly) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  size_t call_count = 0;
  bool key_was_someanchor = false;
  bool evar_init = false;
  bool evar_init_to_correct_vname = false;
  ASSERT_TRUE(v.VerifyAllGoals([&call_count, &key_was_someanchor,
                                &evar_init_to_correct_vname](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    // Check for equivalence to `App(#vname, (#"", #"", 1, #"", #""))`
    key_was_someanchor = (inspection.label == "SomeAnchor");
    if (AstNode *node = inspection.evar->current()) {
      if (App *app = node->AsApp()) {
        if (Tuple *tuple = app->rhs()->AsTuple()) {
          if (app->lhs() == cxt->vname_id() && tuple->size() == 5 &&
              tuple->element(0) == cxt->empty_string_id() &&
              tuple->element(1) == cxt->empty_string_id() &&
              tuple->element(3) == cxt->empty_string_id() &&
              tuple->element(4) == cxt->empty_string_id()) {
            if (Identifier *identifier = tuple->element(2)->AsIdentifier()) {
              evar_init_to_correct_vname =
                  cxt->symbol_table()->text(identifier->symbol()) == "1";
            }
          }
        }
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(key_was_someanchor);
  EXPECT_TRUE(evar_init_to_correct_vname);
}

TEST(VerifierUnitTest, FactsAreNotLinear) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor? defines SomeNode?
#- AnotherAnchor? defines AnotherNode?
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  AstNode *some_anchor = nullptr;
  AstNode *some_node = nullptr;
  AstNode *another_anchor = nullptr;
  AstNode *another_node = nullptr;
  ASSERT_TRUE(v.VerifyAllGoals(
      [&some_anchor, &some_node, &another_anchor, &another_node](
          Verifier *cxt, const AssertionParser::Inspection &inspection) {
        if (AstNode *node = inspection.evar->current()) {
          if (inspection.label == "SomeAnchor") {
            some_anchor = node;
          } else if (inspection.label == "SomeNode") {
            some_node = node;
          } else if (inspection.label == "AnotherAnchor") {
            another_anchor = node;
          } else if (inspection.label == "AnotherNode") {
            another_node = node;
          } else {
            return false;
          }
          return true;
        }
        return false;
      }));
  EXPECT_EQ(some_anchor, another_anchor);
  EXPECT_EQ(some_node, another_node);
  EXPECT_NE(some_anchor, some_node);
  EXPECT_NE(another_anchor, another_node);
}

TEST(VerifierUnitTest, OrdinalsGetUnified) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor is_param.Ordinal? SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param"
target { root:"2" }
fact_name: "/kythe/ordinal"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  size_t call_count = 0;
  bool key_was_ordinal = false;
  bool evar_init = false;
  bool evar_init_to_correct_ordinal = false;
  ASSERT_TRUE(v.VerifyAllGoals([&call_count, &key_was_ordinal, &evar_init,
                                &evar_init_to_correct_ordinal](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    key_was_ordinal = (inspection.label == "Ordinal");
    if (AstNode *node = inspection.evar->current()) {
      evar_init = true;
      if (Identifier *identifier = node->AsIdentifier()) {
        evar_init_to_correct_ordinal =
            cxt->symbol_table()->text(identifier->symbol()) == "42";
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(key_was_ordinal);
  EXPECT_TRUE(evar_init_to_correct_ordinal);
}

TEST(VerifierUnitTest, DotOrdinalsGetUnified) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor is_param.Ordinal? SomeNode
source { root:"1" }
edge_kind: "/kythe/edge/is_param.42"
target { root:"2" }
fact_name: "/"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  size_t call_count = 0;
  bool key_was_ordinal = false;
  bool evar_init = false;
  bool evar_init_to_correct_ordinal = false;
  ASSERT_TRUE(v.VerifyAllGoals([&call_count, &key_was_ordinal, &evar_init,
                                &evar_init_to_correct_ordinal](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    ++call_count;
    key_was_ordinal = (inspection.label == "Ordinal");
    if (AstNode *node = inspection.evar->current()) {
      evar_init = true;
      if (Identifier *identifier = node->AsIdentifier()) {
        evar_init_to_correct_ordinal =
            cxt->symbol_table()->text(identifier->symbol()) == "42";
      }
    }
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(key_was_ordinal);
  EXPECT_TRUE(evar_init_to_correct_ordinal);
}

TEST(VerifierUnitTest, EvarsAndIdentifiersCanHaveTheSameText) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, "Signature", Root?, Path?, Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Signature"
  root:"Root"
  path:"Path"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool root = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &root, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if (ident_content == inspection.label) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Root") root = true;
          if (inspection.label == "Path") path = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(root);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, EvarsAndIdentifiersCanHaveTheSameTextAndAreNotRebound) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, "Signature", Signature?, Path?, Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Signature"
  root:"Signature"
  path:"Path"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if (ident_content == inspection.label) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Path") path = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, EqualityConstraintWorks) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is vname(_,_,Two? = "2",_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals(
      [](Verifier *cxt, const AssertionParser::Inspection &inspection) {
        if (AstNode *node = inspection.evar->current()) {
          if (Identifier *ident = node->AsIdentifier()) {
            return cxt->symbol_table()->text(ident->symbol()) == "2";
          }
        }
        return false;
      }));
}

TEST(VerifierUnitTest, EqualityConstraintWorksOnAnchors) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- Tx?=@text defines SomeNode
##text (line 3 column 2 offset 42-46)
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "42"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "46"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals([](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    return (inspection.label == "Tx" && inspection.evar->current() != nullptr);
  }));
}

// It's possible to match Tx against {root:7}:
TEST(VerifierUnitTest, EqualityConstraintWorksOnAnchorsPossibleConstraint) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
##text (line 3 column 2 offset 38-42)
#- Tx.node/kind notananchor
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"7" }
fact_name: "/kythe/node/kind"
fact_value: "notananchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "38"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "42"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

// It's impossible to match Tx against {root:7} if we constrain Tx to equal
// an anchor specifier.
TEST(VerifierUnitTest, EqualityConstraintWorksOnAnchorsImpossibleConstraint) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- Tx?=@text defines SomeNode
##text (line 3 column 2 offset 42-46)
#- Tx.node/kind notananchor
source { root:"1" }
fact_name: "/kythe/node/kind"
fact_value: "anchor"
}
entries {
source { root:"7" }
fact_name: "/kythe/node/kind"
fact_value: "notananchor"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/start"
fact_value: "42"
}
entries {
source { root:"1" }
fact_name: "/kythe/loc/end"
fact_value: "46"
}
entries {
source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, EqualityConstraintWorksWithOtherSortedRules) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is vname(_,_,Three? = "3",_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals(
      [](Verifier *cxt, const AssertionParser::Inspection &inspection) {
        if (AstNode *node = inspection.evar->current()) {
          if (Identifier *ident = node->AsIdentifier()) {
            return cxt->symbol_table()->text(ident->symbol()) == "3";
          }
        }
        return false;
      }));
}

TEST(VerifierUnitTest, EqualityConstraintFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is vname(_,_,Two = "2",_,_)
#- One is vname(_,_,Three = "3",_,_)
#- One is vname(_,_,Two = Three,_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, IdentityEqualityConstraintSucceeds) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is vname(_,_,Two = Two,_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, TransitiveIdentityEqualityConstraintFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is vname(_,_,Two = Dos = Two,_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, GraphEqualityConstraintFails) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- One is Two = vname(_,_,Two,_,_)
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"2" }
} entries {
source { root:"1" }
fact_name: "/"
fact_value: ""
edge_kind: "/kythe/edge/is"
target { root:"3" }
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, UnifyVersusVname) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, Corpus?, Root?, Path?, Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Corpus"
  root:"Root"
  path:"Path"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool corpus = false;
  bool root = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &corpus, &root, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if (ident_content == inspection.label) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Corpus") corpus = true;
          if (inspection.label == "Root") root = true;
          if (inspection.label == "Path") path = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(corpus);
  EXPECT_TRUE(root);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, UnifyVersusVnameWithDontCare) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, Corpus?, Root?, _?, Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Corpus"
  root:"Root"
  path:"Path"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool corpus = false;
  bool root = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &corpus, &root, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if ((inspection.label != "Path" && ident_content == inspection.label) ||
            (inspection.label == "_" && ident_content == "Path")) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Corpus") corpus = true;
          if (inspection.label == "Root") root = true;
          if (inspection.label == "_") path = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(corpus);
  EXPECT_TRUE(root);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, UnifyVersusVnameWithEmptyStringSpelledOut) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, Corpus?, Root?, "", Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Corpus"
  root:"Root"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool corpus = false;
  bool root = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &corpus, &root, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if (ident_content == inspection.label) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Corpus") corpus = true;
          if (inspection.label == "Root") root = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(corpus);
  EXPECT_TRUE(root);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, UnifyVersusVnameWithEmptyStringBound) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- vname(Signature?, Corpus?, Root?, Path?, Language?) defines SomeNode
source {
  signature:"Signature"
  corpus:"Corpus"
  root:"Root"
  language:"Language"
}
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  bool signature = false;
  bool corpus = false;
  bool root = false;
  bool path = false;
  bool language = false;
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &corpus, &root, &path, &language](
      Verifier *cxt, const AssertionParser::Inspection &inspection) {
    if (AstNode *node = inspection.evar->current()) {
      if (Identifier *ident = node->AsIdentifier()) {
        std::string ident_content = cxt->symbol_table()->text(ident->symbol());
        if ((inspection.label != "Path" && ident_content == inspection.label) ||
            (inspection.label == "Path" && ident == cxt->empty_string_id())) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Corpus") corpus = true;
          if (inspection.label == "Root") root = true;
          if (inspection.label == "Path") path = true;
          if (inspection.label == "Language") language = true;
        }
      }
      return true;
    }
    return false;
  }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(corpus);
  EXPECT_TRUE(root);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST(VerifierUnitTest, LastGoalToFailIsSelected) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 43
#- SomeNode.content 43
#- SomeNode.content 42
#- SomeNode.content 46
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
  ASSERT_EQ(2, v.highest_goal_reached());
}

}  // anonymous namespace
}  // namespace verifier
}  // namespace kythe

int main(int argc, char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  ::google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
