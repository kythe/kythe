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

#include <optional>
#include <regex>
#include <string_view>

#include "absl/log/initialize.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "verifier.h"

namespace kythe {
namespace verifier {
namespace {
using MarkedSource = kythe::proto::common::MarkedSource;

TEST(VerifierUnitTest, StringPrettyPrinter) {
  StringPrettyPrinter c_string;
  c_string.Print("c_string");
  EXPECT_EQ("c_string", c_string.str());
  StringPrettyPrinter std_string;
  std_string.Print(std::string("std_string"));
  EXPECT_EQ("std_string", std_string.str());
  StringPrettyPrinter zero_ptrvoid;
  void* zero = nullptr;
  zero_ptrvoid.Print(zero);
  EXPECT_EQ("0", zero_ptrvoid.str());
  void* one = reinterpret_cast<void*>(1);
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
  void* zero = nullptr;
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
  void* zero = nullptr;
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

bool CheckEVarInit(std::string_view s) { return s != "nil" && !s.empty(); }

enum class Solver { Old, New };

class VerifierTest : public testing::TestWithParam<Solver> {
 protected:
  void SetUp() override {
    v.UseFastSolver(GetParam() == Solver::Old ? false : true);
  }

  Verifier v;
};

INSTANTIATE_TEST_SUITE_P(Solvers, VerifierTest,
                         testing::Values(Solver::Old, Solver::New),
                         [](const auto& p) {
                           return p.param == Solver::Old ? "old" : "new";
                         });

TEST_P(VerifierTest, TrivialHappyCase) { ASSERT_TRUE(v.VerifyAllGoals()); }

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

TEST_P(VerifierTest, NoRulesIsOk) {
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

TEST_P(VerifierTest, EdgesCanSupplyMultipleOrdinals) {
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

TEST_P(VerifierTest, EdgesCanSupplyMultipleDotOrdinals) {
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

TEST_P(VerifierTest, MissingAnchorTextFails) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
}

TEST_P(VerifierTest, AmbiguousAnchorTextFails) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @text defines SomeNode
# text text
  source { root: "1" }
  fact_name: "testname"
  fact_value: "testvalue"
})"));
}

TEST_P(VerifierTest, GenerateAnchorEvarFailsOnEmptyDB) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @text defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, OffsetsVersusRuleBlocks) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @text defines SomeNode
#- @+2text defines SomeNode
#- @+1text defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, ZeroRelativeLineReferencesDontWork) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+0text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, NoMatchingInsideGoalComments) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+1text defines SomeNode
#- @text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, OutOfBoundsRelativeLineReferencesDontWork) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @+2text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, EndOfFileAbsoluteLineReferencesWork) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @:3text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, OutOfBoundsAbsoluteLineReferencesDontWork) {
  Verifier v;
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:4text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, ZeroAbsoluteLineReferencesDontWork) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:0text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, SameAbsoluteLineReferencesDontWork) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#- @:1text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, HistoricalAbsoluteLineReferencesDontWork) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(
#
#- @:1text defines SomeNode
# text
)"));
}

TEST_P(VerifierTest, ParseLiteralString) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text" defines SomeNode
# text
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, ParseLiteralStringWithSpace) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text txet" defines SomeNode
# text txet
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, ParseLiteralStringWithEscape) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
#- @"text \"txet\" ettx" defines SomeNode
# text "txet" ettx
)"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateStartOffsetEVar) {
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

TEST_P(VerifierTest, GenerateStartOffsetEVarRelativeLine) {
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

TEST_P(VerifierTest, GenerateEndOffsetEVarAbsoluteLine) {
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

TEST_P(VerifierTest, GenerateEndOffsetEVar) {
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

TEST_P(VerifierTest, GenerateAnchorEvar) {
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
})",
                                    "", "1"));
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

TEST_P(VerifierTest, GenerateAnchorEvarMatchNumber0) {
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#0text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"1" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph,
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarMatchNumber1) {
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#1text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"3" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph,
                                    "", "3"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarMatchNumber2) {
  ASSERT_TRUE(v.LoadInlineProtoFile(std::string(R"(entries {
#- @#2text defines SomeNode
##text text text(40-44, 45-49, 50-54)

source { root:"4" }
edge_kind: "/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})") + kMatchAnchorSubgraph,
                                    "", "4"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarMatchNumber3) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @#3text defines SomeNode
##text text text(40-44, 45-49, 50-54)
})"));
}

TEST_P(VerifierTest, GenerateAnchorEvarMatchNumberNegative1) {
  ASSERT_FALSE(v.LoadInlineProtoFile(R"(entries {
#- @#-1text defines SomeNode
##text text text(40-44, 45-49, 50-54)
})"));
}

TEST_P(VerifierTest, GenerateAnchorEvarAbsoluteLine) {
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
##text (line 24 column 2 offset 387-391))",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarRelativeLine) {
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
##text (line 24 column 2 offset 387-391))",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarAtEndOfFile) {
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
##text (line 22 column 2 offset 384-388))",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarAtEndOfFileWithSpaces) {
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
##text (line 22 column 2 offset 386-390))",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarAtEndOfFileWithTrailingRule) {
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
#- SomeAnchor defines SomeNode)",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarAcrossMultipleGoalLines) {
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
})",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarAcrossMultipleGoalLinesWithSpaces) {
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
})",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarWithBlankLines) {
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

#-  SomeAnchor defines SomeNode)",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, GenerateAnchorEvarWithWhitespaceLines) {
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

#-  SomeAnchor defines SomeNode)",
                                    "", "1"));
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

TEST_P(VerifierTest, ContentFactPasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, BCPLCommentBlocksWrongRule) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- //SomeNode.content 43
#- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, BCPLCommentInStringLiteral) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content "4//2"
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "4//2"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, BCPLCommentTrailingValue) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42//x
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, EmptyBCPLCommentTrailingValue) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42//
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, EmptyBCPLComment) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#-//
#- SomeNode.content 43
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&call_count, &evar_unset](Verifier* cxt, const Inspection& inspection) {
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&call_count, &evar_unset](Verifier* cxt, const Inspection& inspection) {
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
  ASSERT_FALSE(v.VerifyAllGoals(
      [&call_count, &evar_set](Verifier* cxt, const Inspection& inspection) {
        ++call_count;
        if (inspection.label == "Root" && inspection.evar->current()) {
          if (Identifier* identifier =
                  inspection.evar->current()->AsIdentifier()) {
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
  ASSERT_FALSE(v.VerifyAllGoals(
      [&call_count, &evar_set](Verifier* cxt, const Inspection& inspection) {
        ++call_count;
        if (inspection.label == "Root" && inspection.evar->current()) {
          if (Identifier* identifier =
                  inspection.evar->current()->AsIdentifier()) {
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
  ASSERT_FALSE(v.VerifyAllGoals(
      [&call_count, &evar_set](Verifier* cxt, const Inspection& inspection) {
        ++call_count;
        if (inspection.label == "Root" && inspection.evar->current()) {
          if (Identifier* identifier =
                  inspection.evar->current()->AsIdentifier()) {
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

TEST_P(VerifierTest, GroupFactFails) {
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
TEST_P(VerifierTest, FailWithCutInGroups) {
  if (GetParam() == Solver::New) {
    // The new solver will pick the second entry.
    GTEST_SKIP();
  }
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
TEST_P(VerifierTest, FailWithCut) {
  if (GetParam() == Solver::New) {
    // The new solver will pick the second entry.
    GTEST_SKIP();
  }
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

TEST_P(VerifierTest, PassWithoutCut) {
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

TEST_P(VerifierTest, GroupFactPasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- { SomeNode.content 42 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, CompoundGroups) {
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

TEST_P(VerifierTest, ConjunctionInsideNegatedGroupPassFail) {
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

TEST_P(VerifierTest, ConjunctionInsideNegatedGroupPassPass) {
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

TEST_P(VerifierTest, AntiContentFactFails) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 42 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, AntiContentFactPasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- !{ SomeNode.content 43 }
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, SpacesAreOkay) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
   	 #- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "42"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, ContentFactFails) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, PercentContentFactFails) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeNode.%content 42
source { root:"1" }
fact_name: "%/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, SpacesDontDisableRules) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
   	 #- SomeNode.content 42
source { root:"1" }
fact_name: "/kythe/content"
fact_value: "43"
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, DefinesEdgePasses) {
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

TEST_P(VerifierTest, HashDefinesEdgePasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor #defines SomeNode
source { root:"1" }
edge_kind: "#/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, HashFullDefinesEdgePasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor #/kythe/edge/defines SomeNode
source { root:"1" }
edge_kind: "#/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, PercentDefinesEdgePasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor %defines SomeNode
source { root:"1" }
edge_kind: "%/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, PercentFullDefinesEdgePasses) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- SomeAnchor %/kythe/edge/defines SomeNode
source { root:"1" }
edge_kind: "%/kythe/edge/defines"
target { root:"2" }
fact_name: "/"
fact_value: ""
})"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierTest, IsParamEdgePasses) {
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

TEST_P(VerifierTest, IsParamEdgePassesDotOrdinal) {
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

TEST_P(VerifierTest, IsParamEdgeBadDotOrdinalNoNumber) {
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

TEST_P(VerifierTest, IsParamEdgeBadDotOrdinalOnlyDot) {
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

TEST_P(VerifierTest, IsParamEdgeBadDotOrdinalNoEdge) {
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

TEST_P(VerifierTest, IsParamEdgeFailsOnWrongOrdinal) {
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

TEST_P(VerifierTest, IsParamEdgeFailsOnWrongDotOrdinal) {
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

TEST_P(VerifierTest, IsParamEdgeFailsOnMissingOrdinalInGoal) {
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

TEST_P(VerifierTest, IsParamEdgeFailsOnMissingDotOrdinalInGoal) {
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

TEST_P(VerifierTest, IsParamEdgeFailsOnMissingOrdinalInFact) {
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

TEST_P(VerifierTest, EvarsShareANamespace) {
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

TEST_P(VerifierTest, DefinesEdgePassesSymmetry) {
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

TEST_P(VerifierTest, EvarsStillShareANamespace) {
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
  ASSERT_FALSE(
      v.VerifyAllGoals([](Verifier* cxt, const Inspection&) { return false; }));
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
  EVar* seen_evar = nullptr;
  int seen_count = 0;
  ASSERT_TRUE(v.VerifyAllGoals(
      [&seen_evar, &seen_count](Verifier* cxt, const Inspection& inspection) {
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
  ASSERT_TRUE(
      v.VerifyAllGoals([](Verifier* cxt, const Inspection&) { return true; }));
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
  ASSERT_TRUE(
      v.VerifyAllGoals([&inspect_count](Verifier* cxt, const Inspection&) {
        ++inspect_count;
        return true;
      }));
  ASSERT_EQ(2, inspect_count);
}

TEST_P(VerifierTest, InspectionCalledCorrectly) {
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
  std::string istr;
  ASSERT_TRUE(v.VerifyAllGoals([&](Verifier* cxt, const Inspection& inspection,
                                   std::optional<std::string_view> s) {
    ++call_count;
    // Check for equivalence to `App(#vname, (#"", #"", 1, #"", #""))`
    key_was_someanchor = (inspection.label == "SomeAnchor");
    istr = s ? *s : v.InspectionString(inspection);
    return true;
  }));
  EXPECT_EQ(1, call_count);
  EXPECT_EQ(R"(vname("", "", "1", "", ""))", istr);
  EXPECT_TRUE(key_was_someanchor);
}

TEST_P(VerifierTest, FactsAreNotLinear) {
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
  std::string some_anchor, some_node, another_anchor, another_node;
  ASSERT_TRUE(v.VerifyAllGoals(
      [&some_anchor, &some_node, &another_anchor, &another_node](
          Verifier* cxt, const Inspection& inspection, std::string_view s) {
        if (inspection.label == "SomeAnchor") {
          some_anchor = s;
        } else if (inspection.label == "SomeNode") {
          some_node = s;
        } else if (inspection.label == "AnotherAnchor") {
          another_anchor = s;
        } else if (inspection.label == "AnotherNode") {
          another_node = s;
        } else {
          return false;
        }
        return true;
      }));
  EXPECT_EQ(some_anchor, another_anchor);
  EXPECT_EQ(some_node, another_node);
  EXPECT_NE(some_anchor, some_node);
  EXPECT_NE(another_anchor, another_node);
}

TEST_P(VerifierTest, OrdinalsGetUnified) {
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&call_count, &key_was_ordinal, &evar_init,
       &evar_init_to_correct_ordinal](
          Verifier* cxt, const Inspection& inspection, std::string_view s) {
        ++call_count;
        key_was_ordinal = (inspection.label == "Ordinal");
        evar_init = CheckEVarInit(s);
        evar_init_to_correct_ordinal = s == "\"42\"";
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&call_count, &key_was_ordinal, &evar_init,
       &evar_init_to_correct_ordinal](
          Verifier* cxt, const Inspection& inspection, std::string_view s) {
        ++call_count;
        key_was_ordinal = (inspection.label == "Ordinal");
        evar_init = CheckEVarInit(s);
        evar_init_to_correct_ordinal = s == "\"42\"";
        return true;
      }));
  EXPECT_EQ(1, call_count);
  EXPECT_TRUE(key_was_ordinal);
  EXPECT_TRUE(evar_init_to_correct_ordinal);
}

TEST_P(VerifierTest, EvarsAndIdentifiersCanHaveTheSameText) {
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&signature, &root, &path, &language](
          Verifier* cxt, const Inspection& inspection, std::string_view s) {
        if (inspection.label == "Signature") signature = s == "Signature";
        if (inspection.label == "Root") root = s == "Root";
        if (inspection.label == "Path") path = s == "Path";
        if (inspection.label == "Language") language = s == "Language";
        return true;
      }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(root);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST_P(VerifierTest, EvarsAndIdentifiersCanHaveTheSameTextAndAreNotRebound) {
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
  ASSERT_TRUE(v.VerifyAllGoals(
      [&signature, &path, &language](
          Verifier* cxt, const Inspection& inspection, std::string_view s) {
        if (s == inspection.label) {
          if (inspection.label == "Signature") signature = true;
          if (inspection.label == "Path") path = true;
          if (inspection.label == "Language") language = true;
        } else {
          return false;
        }
        return true;
      }));
  EXPECT_TRUE(signature);
  EXPECT_TRUE(path);
  EXPECT_TRUE(language);
}

TEST_P(VerifierTest, EqualityConstraintWorks) {
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
      [](Verifier* cxt, const Inspection& inspection, absl::string_view s) {
        return (inspection.label == "Two" && s == "\"2\"");
      }));
}

TEST_P(VerifierTest, EqualityConstraintWorksOnAnchors) {
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
})",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals(
      [](Verifier* cxt, const Inspection& inspection, std::string_view s) {
        return (inspection.label == "Tx" && !s.empty());
      }));
}

TEST_P(VerifierTest, EqualityConstraintWorksOnAnchorsRev) {
  if (GetParam() == Solver::New) {
    // TODO: Turns out that we do need to propagate (type) equality constraints.
    GTEST_SKIP();
  }
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
#- @text=Tx? defines SomeNode
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
})",
                                    "", "1"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals(
      [](Verifier* cxt, const Inspection& inspection, std::string_view s) {
        return (inspection.label == "Tx" && !s.empty());
      }));
}

// It's possible to match Tx against {root:7}:
TEST_P(VerifierTest, EqualityConstraintWorksOnAnchorsPossibleConstraint) {
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
})",
                                    "", "1"));
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
  ASSERT_TRUE(v.VerifyAllGoals([](Verifier* cxt, const Inspection& inspection) {
    if (AstNode* node = inspection.evar->current()) {
      if (Identifier* ident = node->AsIdentifier()) {
        return cxt->symbol_table()->text(ident->symbol()) == "3";
      }
    }
    return false;
  }));
}

TEST_P(VerifierTest, EqualityConstraintFails) {
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

TEST_P(VerifierTest, IdentityEqualityConstraintSucceeds) {
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

TEST_P(VerifierTest, TransitiveIdentityEqualityConstraintFails) {
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
  if (GetParam() == Solver::New) {
    // The new solver can handle cycles in equality constraints.
    ASSERT_TRUE(v.VerifyAllGoals());
  } else {
    ASSERT_FALSE(v.VerifyAllGoals());
  }
}

TEST_P(VerifierTest, GraphEqualityConstraintFails) {
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
                                   Verifier* cxt,
                                   const Inspection& inspection) {
    if (AstNode* node = inspection.evar->current()) {
      if (Identifier* ident = node->AsIdentifier()) {
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
                                   Verifier* cxt,
                                   const Inspection& inspection) {
    if (AstNode* node = inspection.evar->current()) {
      if (Identifier* ident = node->AsIdentifier()) {
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
  ASSERT_TRUE(v.VerifyAllGoals([&signature, &corpus, &root, &language](
                                   Verifier* cxt,
                                   const Inspection& inspection) {
    if (AstNode* node = inspection.evar->current()) {
      if (Identifier* ident = node->AsIdentifier()) {
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
                                   Verifier* cxt,
                                   const Inspection& inspection) {
    if (AstNode* node = inspection.evar->current()) {
      if (Identifier* ident = node->AsIdentifier()) {
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

TEST(VerifierUnitTest, ReadGoalsFromFileNodeFailure) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { path:"test" }
fact_name: "/kythe/node/kind"
fact_value: "file"
}
entries {
source { path:"test" }
fact_name: "/kythe/text"
fact_value: "//- A.node/kind file\n//- A.notafact yes\n"
})"));
  v.UseFileNodes();
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
  ASSERT_EQ(1, v.highest_goal_reached());
}

TEST_P(VerifierTest, ReadGoalsFromFileNodeSuccess) {
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { path:"test" }
fact_name: "/kythe/node/kind"
fact_value: "file"
}
entries {
source { path:"test" }
fact_name: "/kythe/text"
fact_value: "//- A.node/kind file\n"
})"));
  v.UseFileNodes();
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST(VerifierUnitTest, ReadGoalsFromFileNodeFailParse) {
  Verifier v;
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(entries {
source { path:"test" }
fact_name: "/kythe/node/kind"
fact_value: "file"
}
entries {
source { path:"test" }
fact_name: "/kythe/text"
fact_value: "//- A->node/kind file\n"
})"));
  v.UseFileNodes();
  ASSERT_FALSE(v.PrepareDatabase());
}

TEST(VerifierUnitTest, DontConvertMarkedSource) {
  Verifier v;
  MarkedSource source;
  std::string source_string;
  ASSERT_TRUE(source.SerializeToString(&source_string));
  google::protobuf::TextFormat::FieldValuePrinter printer;
  std::string enc_source = printer.PrintBytes(source_string);
  ASSERT_TRUE(v.LoadInlineProtoFile(R"(
  entries {
    source { signature:"test" }
    fact_name: "/kythe/code"
    fact_value: )" + enc_source + R"(
  }
  #- vname("test","","","","").code _
  #- !{vname("test","","","","") code _}
  )"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

enum class MsFormat { kTextProto, kJson };

class VerifierMarkedSourceUnitTest
    : public ::testing::TestWithParam<std::tuple<MsFormat, Solver>> {
 protected:
  std::string Entries(const MarkedSource& source) const {
    kythe::proto::Entries entries;
    auto entry = entries.add_entries();
    entry->mutable_source()->set_signature("test");
    entry->set_fact_name(FactName());
    entry->set_fact_value(Encode(source));
    std::string result;
    GTEST_CHECK_(google::protobuf::TextFormat::PrintToString(entries, &result));
    return result;
  }

  void ConfigureVerifier(Verifier& v) {
    v.UseFastSolver(std::get<1>(GetParam()) == Solver::Old ? false : true);
  }

 private:
  std::string FactName() const {
    return std::get<0>(GetParam()) == MsFormat::kTextProto ? "/kythe/code"
                                                           : "/kythe/code/json";
  }

  std::string Encode(const MarkedSource& source) const {
    if (std::get<0>(GetParam()) == MsFormat::kTextProto) {
      std::string source_string;
      GTEST_CHECK_(source.SerializeToString(&source_string))
          << "Failure serializing MarkedSource";
      return source_string;
    } else {
      std::string source_string;
      GTEST_CHECK_(
          google::protobuf::util::MessageToJsonString(source, &source_string)
              .ok())
          << "Failure serializing MarkedSource to JSON";
      return source_string;
    }
  }
};

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSource) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + R"(
  #- !{vname("test","","","","").code _}
  #- vname("test","","","","") code Tree
  #- Tree.kind "BOX"
  #- Tree.pre_text ""
  #- Tree.post_child_text ""
  #- Tree.post_text ""
  #- Tree.lookup_index 0
  #- Tree.default_children_count 0
  #- Tree.add_final_list_token false
  )"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceKindEnums) {
  for (int kind = MarkedSource::Kind_MIN; kind <= MarkedSource::Kind_MAX;
       ++kind) {
    if (!MarkedSource::Kind_IsValid(kind)) {
      continue;
    }
    auto kind_enum = static_cast<MarkedSource::Kind>(kind);
    Verifier v;
    ConfigureVerifier(v);
    v.ConvertMarkedSource();
    MarkedSource source;
    source.set_kind(kind_enum);
    ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + R"(
    #- vname("test","","","","") code Tree
    #- Tree.kind ")" + MarkedSource::Kind_Name(kind_enum) +
                                      R"("
    )"));
    ASSERT_TRUE(v.PrepareDatabase());
    ASSERT_TRUE(v.VerifyAllGoals());
  }
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceLinks) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  source.add_link()->add_definition("kythe://corpus#sig");
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + R"(
  #- vname("test","","","","") code Tree
  #- Tree link vname("sig", "corpus", "", "", "")
  )"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceLinksBadUri) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  source.add_link()->add_definition("kythe:/&bad");
  ASSERT_FALSE(v.LoadInlineProtoFile(Entries(source)));
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceLinksMissingUri) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  source.add_link();
  ASSERT_FALSE(v.LoadInlineProtoFile(Entries(source)));
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceLinksMultipleUri) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  auto* link = source.add_link();
  link->add_definition("kythe://corpus#sig");
  link->add_definition("kythe://corpus#sig2");
  ASSERT_FALSE(v.LoadInlineProtoFile(Entries(source)));
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceChildren) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  auto* child = source.add_child();
  child->set_kind(MarkedSource::IDENTIFIER);
  child = source.add_child();
  child->set_kind(MarkedSource::CONTEXT);
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + R"(
  #- vname("test","","","","") code Tree
  #- Tree child.0 IdChild
  #- IdChild.kind "IDENTIFIER"
  #- Tree child.1 ContextChild
  #- ContextChild.kind "CONTEXT"
  )"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceBadChildren) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource parent;
  MarkedSource* child = parent.add_child();
  auto* link = child->add_link();
  link->add_definition("kythe://corpus#sig");
  link->add_definition("kythe://corpus#sig2");
  ASSERT_FALSE(v.LoadInlineProtoFile(Entries(parent)));
}

TEST_P(VerifierMarkedSourceUnitTest, ConvertMarkedSourceFields) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  source.set_pre_text("pre_text");
  source.set_post_child_text("post_child_text");
  source.set_post_text("post_text");
  source.set_lookup_index(42);
  source.set_default_children_count(43);
  source.set_add_final_list_token(true);
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + R"(
  #- vname("test","","","","") code Tree
  #- Tree.pre_text "pre_text"
  #- Tree.post_child_text "post_child_text"
  #- Tree.post_text "post_text"
  #- Tree.lookup_index 42
  #- Tree.default_children_count 43
  #- Tree.add_final_list_token true
  )"));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest,
       DontConvertMarkedSourceDuplicateFactsWellFormed) {
  Verifier v;
  ConfigureVerifier(v);
  MarkedSource source;
  v.IgnoreDuplicateFacts();
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + "\n" + Entries(source)));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest,
       ConvertMarkedSourceDuplicateFactsWellFormed) {
  Verifier v;
  ConfigureVerifier(v);
  v.ConvertMarkedSource();
  MarkedSource source;
  ASSERT_TRUE(v.LoadInlineProtoFile(Entries(source) + "\n" + Entries(source)));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest, ConflictingCodeFactsNotWellFormed) {
  if (std::get<1>(GetParam()) == Solver::New) {
    // The new solver doesn't currently perform conflict checks.
    GTEST_SKIP();
  }
  Verifier v;
  ConfigureVerifier(v);
  MarkedSource source;

  MarkedSource source_conflict;
  source_conflict.set_pre_text("pre_text");
  ASSERT_TRUE(
      v.LoadInlineProtoFile(Entries(source) + "\n" + Entries(source_conflict)));
  ASSERT_FALSE(v.PrepareDatabase());
  ASSERT_FALSE(v.VerifyAllGoals());
}

TEST_P(VerifierMarkedSourceUnitTest, ConflictingCodeFactsIgnoreWellFormed) {
  Verifier v;
  ConfigureVerifier(v);
  v.IgnoreCodeConflicts();
  MarkedSource source;
  MarkedSource source_conflict;
  source_conflict.set_pre_text("pre_text");
  ASSERT_TRUE(
      v.LoadInlineProtoFile(Entries(source) + "\n" + Entries(source_conflict)));
  ASSERT_TRUE(v.PrepareDatabase());
  ASSERT_TRUE(v.VerifyAllGoals());
}

INSTANTIATE_TEST_SUITE_P(
    JsonAndProto, VerifierMarkedSourceUnitTest,
    ::testing::Combine(::testing::Values(MsFormat::kTextProto, MsFormat::kJson),
                       ::testing::Values(Solver::Old, Solver::New)),
    [](const auto& p) {
      if (std::get<0>(p.param) == MsFormat::kTextProto)
        return std::get<1>(p.param) == Solver::Old ? "textold" : "textnew";
      else
        return std::get<1>(p.param) == Solver::Old ? "jsonold" : "jsonnew";
    });

}  // anonymous namespace
}  // namespace verifier
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  ::absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
