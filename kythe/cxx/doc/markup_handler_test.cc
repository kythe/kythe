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

#include "kythe/cxx/doc/markup_handler.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/doc/javadoc_markup_handler.h"

#include <stack>

namespace kythe {
namespace {
void ReturnNoSpans(const Printable& in_message, PrintableSpans* out_spans) {}
class MarkupHandlerTest : public ::testing::Test {
 public:
 protected:
  void Handle(const std::string& raw_text, const MarkupHandler& handler,
              const std::string& bracketed_expected,
              std::initializer_list<PrintableSpan> spans) {
    reply_.set_raw_text(raw_text);
    Printable input(reply_);
    auto output = HandleMarkup({handler}, input);
    auto bracketed_actual = Bracket(output);
    EXPECT_EQ(bracketed_expected, bracketed_actual);
    EXPECT_EQ(spans.size(), output.spans().size());
    size_t actual_span = 0;
    for (const auto& span : spans) {
      if (actual_span >= output.spans().size()) {
        break;
      }
      const auto& aspan = output.spans().span(actual_span++);
      EXPECT_EQ(span.semantic(), aspan.semantic());
      if (span.semantic() == aspan.semantic()) {
        switch (span.semantic()) {
          case PrintableSpan::Semantic::Link: {
            ASSERT_EQ(1, span.link().definition_size());
            EXPECT_EQ(span.link().definition_size(),
                      aspan.link().definition_size());
            if (span.link().definition_size() == 1) {
              EXPECT_EQ(span.link().definition(0).parent(),
                        aspan.link().definition(0).parent());
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }
  std::string Bracket(const Printable& printable) {
    // NB: this doesn't do any escaping.
    std::string result;
    result.reserve(printable.text().size() + printable.spans().size() * 2);
    size_t span = 0;
    std::stack<size_t> ends;
    for (size_t i = 0; i < printable.text().size(); ++i) {
      while (!ends.empty() && ends.top() == i) {
        result.push_back(']');
        ends.pop();
      }
      while (span < printable.spans().size() &&
             printable.spans().span(span).begin() == i) {
        result.push_back('[');
        ends.push(printable.spans().span(span).end());
        ++span;
      }
      result.push_back(printable.text()[i]);
    }
    while (!ends.empty()) {
      result.push_back(']');
      ends.pop();
    }
    return result;
  }
  proto::DocumentationReply_Link& ExpectLink(const std::string& uri) {
    auto* link = reply_.add_link();
    auto* definition = link->add_definition();
    definition->set_parent(uri);
    return *link;
  }
  proto::DocumentationReply_Printable reply_;
};
TEST_F(MarkupHandlerTest, Empty) { Handle("", ReturnNoSpans, "", {}); }
TEST_F(MarkupHandlerTest, PassThroughLinks) {
  Handle("[hello]", ReturnNoSpans, "[hello]",
         {PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, JavadocReturns) {
  Handle("@return something", ParseJavadoc, "[[@return][ something]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Html),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Return)});
}
TEST_F(MarkupHandlerTest, JavadocReturnsMultiline) {
  Handle(R"(@return something
else)",
         ParseJavadoc, R"([[@return][ something
else]])",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Html),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Return)});
}
TEST_F(MarkupHandlerTest, JavadocReturnsLink) {
  Handle("@return [something]", ParseJavadoc, "[[@return][ [something]]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Html),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Return),
          PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, JavadocReturnsLinkSortOrder) {
  Handle("@return[ something]", ParseJavadoc, "[[@return][[ something]]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Html),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Return),
          PrintableSpan(0, 0, ExpectLink("uri"))});
}
}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
