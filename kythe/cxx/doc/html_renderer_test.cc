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

#include "kythe/cxx/doc/html_renderer.h"

#include "absl/log/initialize.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/doc/html_markup_handler.h"
#include "kythe/cxx/doc/javadoxygen_markup_handler.h"
#include "kythe/proto/common.pb.h"

namespace kythe {
namespace {
using google::protobuf::TextFormat;
class TestHtmlRendererOptions : public HtmlRendererOptions {
 public:
  TestHtmlRendererOptions() {
    node_info_["kythe://foo"].set_definition("kythe://food");
    node_info_["kythe://bar"].set_definition("kythe://bard");
    definition_locations_["kythe://food"].set_parent("kythe://foop");
    definition_locations_["kythe://bard"].set_parent("kythe://barp&q=1<");
  }
  const proto::common::NodeInfo* node_info(
      const std::string& ticket) const override {
    auto record = node_info_.find(ticket);
    return record != node_info_.end() ? &record->second : nullptr;
  }
  const proto::Anchor* anchor_for_ticket(
      const std::string& ticket) const override {
    auto record = definition_locations_.find(ticket);
    return record != definition_locations_.end() ? &record->second : nullptr;
  }

 private:
  std::map<std::string, proto::common::NodeInfo> node_info_;
  std::map<std::string, proto::Anchor> definition_locations_;
};
class HtmlRendererTest : public ::testing::Test {
 public:
  HtmlRendererTest() {
    options_.make_link_uri = [](const proto::Anchor& anchor) {
      return anchor.parent();
    };
  }

 protected:
  std::string RenderAsciiProtoDocument(const char* document_pb) {
    proto::DocumentationReply::Document document;
    if (!TextFormat::ParseFromString(document_pb, &document)) {
      return "(invalid ascii protobuf)";
    }
    Printable printable(document.text());
    return kythe::RenderHtml(options_, printable);
  }
  std::string RenderJavadoc(const char* raw_text) {
    proto::Printable reply_;
    reply_.set_raw_text(raw_text);
    Printable input(reply_);
    auto output = HandleMarkup({ParseJavadoxygen}, input);
    return kythe::RenderHtml(options_, output);
  }
  std::string RenderHtml(const char* raw_text) {
    proto::Printable reply_;
    reply_.set_raw_text(raw_text);
    Printable input(reply_);
    auto output = HandleMarkup({ParseHtml}, input);
    return kythe::RenderHtml(options_, output);
  }
  kythe::TestHtmlRendererOptions options_;
};
TEST_F(HtmlRendererTest, RenderEmptyDoc) {
  EXPECT_EQ("", RenderAsciiProtoDocument(""));
}
TEST_F(HtmlRendererTest, RenderSimpleDoc) {
  EXPECT_EQ("Hello, world!", RenderAsciiProtoDocument(R"(
      text { raw_text: "Hello, world!" }
  )"));
}
TEST_F(HtmlRendererTest, RenderLink) {
  EXPECT_EQ(R"(Hello, <a href="kythe://foop">world</a>!)",
            RenderAsciiProtoDocument(R"(
      text {
        raw_text: "Hello, [world]!"
        link: { definition: "kythe://foo" }
      }
  )"));
}
TEST_F(HtmlRendererTest, DropMissingLink) {
  EXPECT_EQ(R"(Hello, world!)", RenderAsciiProtoDocument(R"(
      text {
        raw_text: "Hello, [world]!"
        link: { definition: "kythe://baz" }
      }
  )"));
}
TEST_F(HtmlRendererTest, EscapeLink) {
  EXPECT_EQ(R"(Hello, <a href="kythe://barp&amp;q=1&lt;">world</a>!)",
            RenderAsciiProtoDocument(R"(
      text {
        raw_text: "Hello, [world]!"
        link: { definition: "kythe://bar" }
      }
  )"));
}
TEST_F(HtmlRendererTest, RenderLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://foop">Hello</a>, <a href="kythe://barp&amp;q=1&lt;">world</a>!)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world][!]"
        link: { definition: "kythe://foo" }
        link: { definition: "kythe://bar" }
      }
  )"));
}
TEST_F(HtmlRendererTest, SkipMissingLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://foop">Hello</a>, world<a href="kythe://barp&amp;q=1&lt;">!</a>)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world][!]"
        link: { definition: "kythe://foo" }
        link: { }
        link: { definition: "kythe://bar" }
      }
  )"));
}
TEST_F(HtmlRendererTest, SkipNestedMissingLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://foop">Hello</a>, world<a href="kythe://barp&amp;q=1&lt;">!</a>)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world[!]]"
        link: { definition: "kythe://foo" }
        link: { }
        link: { definition: "kythe://bar" }
      }
  )"));
}
TEST_F(HtmlRendererTest, EscapeHtml) {
  EXPECT_EQ("&lt;&gt;&amp;&lt;&gt;&amp;[]\\", RenderAsciiProtoDocument(R"(
      text { raw_text: "<>&\\<\\>\\&\\[\\]\\\\" }
  )"));
}
TEST_F(HtmlRendererTest, JavadocTagBlocks) {
  EXPECT_EQ(
      "text\n<div class=\"kythe-doc-tag-section-title\">Author</div>"
      "<div class=\"kythe-doc-tag-section-content\"><ul><li> a\n</li>"
      "<li> b</li></ul></div>",
      RenderJavadoc(R"(text
@author a
@author b)"));
}
TEST_F(HtmlRendererTest, JavadocTagBlockEmbedsCodeRef) {
  EXPECT_EQ(
      "text\n<div class=\"kythe-doc-tag-section-title\">Author</div>"
      "<div class=\"kythe-doc-tag-section-content\"><ul><li> a <tt> robot</tt>"
      "\n</li><li> b</li></ul></div>",
      RenderJavadoc(R"(text
@author a {@code robot}
@author b)"));
}
TEST_F(HtmlRendererTest, EmptyTags) {
  EXPECT_EQ("", RenderHtml("<I></I>"));
  EXPECT_EQ("<i></i>", RenderHtml("<I><B></B></I>"));
}
TEST_F(HtmlRendererTest, PassThroughStyles) {
  EXPECT_EQ(
      "<b><i><h1><h2><h3><h4><h5><h6>x</h6></h5></h4></h3></h2></h1></i></b>",
      RenderHtml("<B><I><H1><H2><H3><H4><H5><H6>x</H6></H5></H4></H3></H2></"
                 "H1></I></B>"));
}
TEST_F(HtmlRendererTest, PassThroughMoreStyles) {
  EXPECT_EQ(
      "<blockquote><small><tt><tt><big><ul><sub><sup>x</sup></sub></ul></big></"
      "tt></tt></small></blockquote>",
      RenderHtml("<BLOCKQUOTE><SMALL><TT><CODE><BIG><UL><SUB><SUP>x</SUP></"
                 "SUB></UL></BIG></CODE></TT></SMALL></BLOCKQUOTE>"));
}
TEST_F(HtmlRendererTest, PassThroughEntities) {
  EXPECT_EQ("&foo;", RenderHtml("&foo;"));
  EXPECT_EQ("&amp;&lt;bar&gt;;", RenderHtml("&<bar>;"));
}
TEST_F(HtmlRendererTest, RenderHtmlLinks) {
  EXPECT_EQ("<a href=\"foo.html\">bar</a>",
            RenderHtml("<A HREF = \"foo.html\" >bar</A>"));
  EXPECT_EQ("<a href=\"&quot;foo.html&quot;\">bar</a>",
            RenderHtml("<A HREF = \"&quot;foo.html&quot;\" >bar</A>"));
  EXPECT_EQ("&lt;A HREF = \"&amp; q;foo.html&amp; q;\" &gt;bar",
            RenderHtml("<A HREF = \"& q;foo.html& q;\" >bar</A>"));
  EXPECT_EQ("&lt;A HREF = \"href=\"foo.html\">\" BAD&gt;bar",
            RenderHtml("<A HREF = \"foo.html\" BAD>bar</A>"));
}
constexpr char kSampleMarkedSource[] = R""(child {
  child {
    kind: CONTEXT
    child {
      kind: IDENTIFIER
      pre_text: "namespace"
    }
    child {
      kind: IDENTIFIER
      pre_text: "(anonymous namespace)"
    }
    child {
      kind: IDENTIFIER
      pre_text: "ClassContainer"
    }
    post_child_text: "::"
    add_final_list_token: true
  }
  child {
    kind: IDENTIFIER
    pre_text: "FunctionName"
  }
}
child {
  kind: PARAMETER
  pre_text: "("
  child {
    post_child_text: " "
    child {
      kind: TYPE
      pre_text: "TypeOne*"
    }
    child {
      child {
        kind: CONTEXT
        child {
          kind: IDENTIFIER
          pre_text: "namespace"
        }
        child {
          kind: IDENTIFIER
          pre_text: "(anonymous namespace)"
        }
        child {
          kind: IDENTIFIER
          pre_text: "ClassContainer"
        }
        child {
          kind: IDENTIFIER
          pre_text: "FunctionName"
        }
        post_child_text: "::"
        add_final_list_token: true
      }
      child {
        kind: IDENTIFIER
        pre_text: "param_name_one"
      }
    }
  }
  child {
    post_child_text: " "
    child {
      kind: TYPE
      pre_text: "TypeTwo*"
    }
    child {
      child {
        kind: CONTEXT
        child {
          kind: IDENTIFIER
          pre_text: "namespace"
        }
        child {
          kind: IDENTIFIER
          pre_text: "(anonymous namespace)"
        }
        child {
          kind: IDENTIFIER
          pre_text: "ClassContainer"
        }
        child {
          kind: IDENTIFIER
          pre_text: "FunctionName"
        }
        post_child_text: "::"
        add_final_list_token: true
      }
      child {
        kind: IDENTIFIER
        pre_text: "param_name_two"
      }
    }
  }
  post_child_text: ", "
  post_text: ")"
}
)"";

TEST_F(HtmlRendererTest, RenderSimpleParams) {
  proto::common::MarkedSource marked;
  ASSERT_TRUE(TextFormat::ParseFromString(kSampleMarkedSource, &marked))
      << "(invalid ascii protobuf)";
  auto params = kythe::RenderSimpleParams(marked);
  ASSERT_EQ(2, params.size());
  EXPECT_EQ("param_name_one", params[0]);
  EXPECT_EQ("param_name_two", params[1]);
}
TEST_F(HtmlRendererTest, RenderSimpleIdentifier) {
  proto::common::MarkedSource marked;
  ASSERT_TRUE(TextFormat::ParseFromString(kSampleMarkedSource, &marked))
      << "(invalid ascii protobuf)";
  EXPECT_EQ("FunctionName", kythe::RenderSimpleIdentifier(marked));
}
TEST_F(HtmlRendererTest, RenderSimpleQualifiedName) {
  proto::common::MarkedSource marked;
  ASSERT_TRUE(TextFormat::ParseFromString(kSampleMarkedSource, &marked))
      << "(invalid ascii protobuf)";
  EXPECT_EQ("namespace::(anonymous namespace)::ClassContainer",
            kythe::RenderSimpleQualifiedName(marked, false));
  EXPECT_EQ("namespace::(anonymous namespace)::ClassContainer::FunctionName",
            kythe::RenderSimpleQualifiedName(marked, true));
}

constexpr char kGoMarkedSource[] = R""(
	kind: PARAMETER
  child {
    kind: TYPE
    pre_text: "*pkg.receiver"
	}
  child {
		kind: BOX
		post_child_text: "."
    child {
      kind: BOX
			child {
        kind: CONTEXT
        pre_text: "pkg"
      }
			child {
			  kind: IDENTIFIER
				pre_text: "param"
			}
    }
    child {
      kind: BOX
      pre_text: " "
    }
    child {
      kind: TYPE
      pre_text: "string"
    }
	}
)"";

TEST_F(HtmlRendererTest, RenderSimpleParamsGo) {
  proto::common::MarkedSource marked;
  ASSERT_TRUE(TextFormat::ParseFromString(kGoMarkedSource, &marked))
      << "(invalid ascii protobuf)";
  auto params = kythe::RenderSimpleParams(marked);
  ASSERT_EQ(2, params.size());
  EXPECT_EQ("param", params[1]);
}
}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
