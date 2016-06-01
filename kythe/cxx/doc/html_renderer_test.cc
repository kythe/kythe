/*
 * Copyright 2016 Google Inc. All rights reserved.
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
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {
class HtmlRendererTest : public ::testing::Test {
 public:
  HtmlRendererTest() {
    options_.make_link_uri = [](const proto::Anchor &anchor) {
      return anchor.parent();
    };
  }

 protected:
  std::string RenderAsciiProtoDocument(const char *document_pb) {
    proto::DocumentationReply_Document document;
    if (!google::protobuf::TextFormat::ParseFromString(document_pb,
                                                       &document)) {
      return "(invalid ascii protobuf)";
    }
    Printable printable(document.text());
    return RenderHtml(options_, printable);
  }
  kythe::HtmlRendererOptions options_;
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
  EXPECT_EQ(R"(Hello, <a href="kythe://world">world</a>!)",
            RenderAsciiProtoDocument(R"(
      text {
        raw_text: "Hello, [world]!"
        link: { definition: { parent: "kythe://world" } }
      }
  )"));
}
TEST_F(HtmlRendererTest, EscapeLink) {
  EXPECT_EQ(R"(Hello, <a href="kythe://world&amp;q=1&lt;">world</a>!)",
            RenderAsciiProtoDocument(R"(
      text {
        raw_text: "Hello, [world]!"
        link: { definition: { parent: "kythe://world&q=1<" } }
      }
  )"));
}
TEST_F(HtmlRendererTest, RenderLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://hello">Hello</a>, <a href="kythe://world">world</a>!)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world][!]"
        link: { definition: { parent: "kythe://hello" } }
        link: { definition: { parent: "kythe://world" } }
      }
  )"));
}
TEST_F(HtmlRendererTest, SkipMissingLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://hello">Hello</a>, world<a href="kythe://bang">!</a>)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world][!]"
        link: { definition: { parent: "kythe://hello" } }
        link: { }
        link: { definition: { parent: "kythe://bang" } }
      }
  )"));
}
TEST_F(HtmlRendererTest, SkipNestedMissingLinks) {
  EXPECT_EQ(
      R"(<a href="kythe://hello">Hello</a>, world<a href="kythe://bang">!</a>)",
      RenderAsciiProtoDocument(R"(
      text {
        raw_text: "[Hello], [world[!]]"
        link: { definition: { parent: "kythe://hello" } }
        link: { }
        link: { definition: { parent: "kythe://bang" } }
      }
  )"));
}
TEST_F(HtmlRendererTest, EscapeHtml) {
  EXPECT_EQ("&lt;&gt;&amp;&lt;&gt;&amp;[]\\", RenderAsciiProtoDocument(R"(
      text { raw_text: "<>&\\<\\>\\&\\[\\]\\\\" }
  )"));
}
}  // anonymous namespace
}  // namespace kythe

int main(int argc, char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
