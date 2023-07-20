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

#include "kythe/cxx/doc/markup_handler.h"

#include <stack>

#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/doc/html_markup_handler.h"
#include "kythe/cxx/doc/javadoxygen_markup_handler.h"

namespace kythe {
namespace {
void ReturnNoSpans(const Printable& in_message, const PrintableSpans&,
                   PrintableSpans* out_spans) {}
class MarkupHandlerTest : public ::testing::Test {
 public:
 protected:
  void Handle(const std::string& raw_text, const MarkupHandler& handler,
              const std::string& dump_expected) {
    reply_.set_raw_text(raw_text);
    Printable input(reply_);
    auto output = HandleMarkup({handler}, input);
    EXPECT_EQ(dump_expected, output.spans().Dump(output.text()));
  }
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
              EXPECT_EQ(span.link().definition(0), aspan.link().definition(0));
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
  proto::common::Link& ExpectLink(const std::string& uri) {
    auto* link = reply_.add_link();
    link->add_definition(uri);
    return *link;
  }
  proto::Printable reply_;
};
TEST_F(MarkupHandlerTest, Empty) { Handle("", ReturnNoSpans, "", {}); }
TEST_F(MarkupHandlerTest, PassThroughLinks) {
  Handle("[hello]", ReturnNoSpans, "[hello]",
         {PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, JavadocReturns) {
  Handle("@return something", ParseJavadoxygen,
         "[[^ @return][tbReturns0  something]]");
}
TEST_F(MarkupHandlerTest, JavadocParam) {
  Handle("Some text.\n@param argument option", ParseJavadoxygen,
         "[Some text.\n[^ @param][tbParam0  argument option]]");
}
TEST_F(MarkupHandlerTest, JavadocAuthors) {
  Handle("@author Aa Bb\n@author Cc Dd", ParseJavadoxygen,
         "[[^ @author][tbAuthor0  Aa Bb\n][^ @author][tbAuthor1  Cc Dd]]");
}
TEST_F(MarkupHandlerTest, JavadocAuthorsEmail) {
  Handle("@author aa@bb.com\n@author cc@dd.com (Cc Dd)", ParseJavadoxygen,
         "[[^ @author][tbAuthor0  aa@bb.com\n][^ @author][tbAuthor1  cc@dd.com "
         "(Cc Dd)]]");
}
TEST_F(MarkupHandlerTest, JavadocAuthorsNewline) {
  Handle("@author Aa Bb\n@author Cc Dd", ParseJavadoxygen,
         "[[^ @author][tbAuthor0  Aa Bb\n][^ @author][tbAuthor1  Cc Dd]]");
}
TEST_F(MarkupHandlerTest, JavadocReturnsMultiline) {
  Handle("@return something\nelse", ParseJavadoxygen,
         "[[^ @return][tbReturns0  something\nelse]]");
}
TEST_F(MarkupHandlerTest, JavadocReturnsLink) {
  Handle("@return [something]", ParseJavadoxygen, "[@return][ [something]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::TagBlockId::Returns, 0),
          PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, JavadocReturnsLinkSortOrder) {
  Handle("@return[ something]", ParseJavadoxygen, "[@return][[ something]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::TagBlockId::Returns, 0),
          PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, DoxygenReturns) {
  Handle("\\\\return[ something]", ParseJavadoxygen, "[\\return][[ something]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::TagBlockId::Returns, 0),
          PrintableSpan(0, 0, ExpectLink("uri"))});
}
TEST_F(MarkupHandlerTest, DoxygenCode) {
  Handle("\\\\c code not", ParseJavadoxygen, "[[^ \\c][coderef  code] not]");
}
TEST_F(MarkupHandlerTest, DoxygenReturnsCode) {
  Handle("\\\\return[ \\\\c something]", ParseJavadoxygen,
         "[\\return][[ [\\c][ something]]]",
         {PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::TagBlockId::Returns, 0),
          PrintableSpan(0, 0, ExpectLink("uri")),
          PrintableSpan(0, 0, PrintableSpan::Semantic::Markup),
          PrintableSpan(0, 0, PrintableSpan::Semantic::CodeRef)});
}
TEST_F(MarkupHandlerTest, HtmlStyles) {
  Handle("<i>x</i>", ParseHtml, "[[^ <i>][sI x][^ </i>]]");
  Handle("<b>x</b>", ParseHtml, "[[^ <b>][sB x][^ </b>]]");
  Handle("<h1>x</h1>", ParseHtml, "[[^ <h1>][sH1 x][^ </h1>]]");
  Handle("<h2>x</h2>", ParseHtml, "[[^ <h2>][sH2 x][^ </h2>]]");
  Handle("<h3>x</h3>", ParseHtml, "[[^ <h3>][sH3 x][^ </h3>]]");
  Handle("<h4>x</h4>", ParseHtml, "[[^ <h4>][sH4 x][^ </h4>]]");
  Handle("<h5>x</h5>", ParseHtml, "[[^ <h5>][sH5 x][^ </h5>]]");
  Handle("<h6>x</h6>", ParseHtml, "[[^ <h6>][sH6 x][^ </h6>]]");
}
TEST_F(MarkupHandlerTest, HtmlParsesLinks) {
  Handle("<a href=\"h.html\">h</a>", ParseHtml,
         "[[uril [^ <a href=\"[uri h.html]\">]h[^ </a>]]]");
  Handle("<a    href = \"h.html\"  >h< / a >", ParseHtml,
         "[[uril [^ <a    href = \"[uri h.html]\"  >]h[^ < / a >]]]");
}
TEST_F(MarkupHandlerTest, HtmlIgnoresBadAttributes) {
  Handle("<a hdef=\"foo\">", ParseHtml, "[<a hdef=\"foo\">]");
  Handle("<a href=\"foo\" special>", ParseHtml,
         "[<a href=\"[uri foo]\" special>]");
  Handle("<a b>", ParseHtml, "[<a b>]");
  Handle("<a hdef=\"foo\">q</a>", ParseHtml, "[<a hdef=\"foo\">q[^ </a>]]");
  Handle("<a href=\"foo\" special>q</a>", ParseHtml,
         "[<a href=\"[uri foo]\" special>q[^ </a>]]");
  Handle("<a b>q</a>", ParseHtml, "[<a b>q[^ </a>]]");
}
TEST_F(MarkupHandlerTest, HtmlIgnoresUnknownTags) {
  Handle("<ayy>", ParseHtml, "[<ayy>]");
}
TEST_F(MarkupHandlerTest, HtmlSelectsParagraphs) {
  Handle("one<p>two<p>three", ParseHtml, "[one[^ <p>][p two][^ <p>][p three]]");
  Handle("one<p>two</p><p>three</p>", ParseHtml,
         "[one[^ <p>][p two[^ </p>]][^ <p>][p three[^ </p>]]]");
  Handle("<p>one<p>two<p>three", ParseHtml,
         "[[^ <p>][p one][^ <p>][p two][^ <p>][p three]]");
}
TEST_F(MarkupHandlerTest, HtmlHandlesLists) {
  Handle("<ul><li>a<li>b<li>c</ul>", ParseHtml,
         "[[^ <ul>][ul [^ <li>][li a][^ <li>][li b][^ <li>][li c]][^ </ul>]]");
  Handle("<ul><li>a</li><li>b</li><li>c", ParseHtml,
         "[[^ <ul>][ul [^ <li>][li a[^ </li>]][^ <li>][li b[^ </li>]][^ "
         "<li>][li c]]]");
}
TEST_F(MarkupHandlerTest, HtmlHandlesMultilevelLists) {
  Handle("<ul><li><ul><li>a</ul><li>b</ul>", ParseHtml,
         "[[^ <ul>][ul [^ <li>][li [^ <ul>][ul [^ <li>][li a]][^ </ul>]][^ "
         "<li>][li b]][^ </ul>]]");
}
TEST_F(MarkupHandlerTest, HtmlInterleavesParagraphsAndLists) {
  Handle(
      "a<p>b<ul><li>hello</ul>c<p>d", ParseHtml,
      "[a[^ <p>][p b[^ <ul>][ul [^ <li>][li hello]][^ </ul>]c][^ <p>][p d]]");
  Handle("a<p>b<ul><li>c<p>d</ul>e<p>f", ParseHtml,
         "[a[^ <p>][p b[^ <ul>][ul [^ <li>][li c[^ <p>][p d]]][^ </ul>]e][^ "
         "<p>][p f]]");
}
TEST_F(MarkupHandlerTest, HtmlEmitsPreBlock) {
  Handle("<pre>hello</pre>", ParseHtml, "[[^ <pre>][cb hello][^ </pre>]]");
  Handle("<pre>hello", ParseHtml, "[[^ <pre>][cb hello]]");
  // It's still HTML inside <pre> blocks.
  Handle("<pre><i>p</i></pre>", ParseHtml,
         "[[^ <pre>][cb [^ <i>][sI p][^ </i>]][^ </pre>]]");
}
TEST_F(MarkupHandlerTest, HtmlHandlesEscapes) {
  Handle("&gt;&quot;&amp;&lt;", ParseHtml,
         "[[esc &gt;][esc &quot;][esc &amp;][esc &lt;]]");
  Handle("<a href=\"&quot;\">quotes</a>", ParseHtml,
         "[[uril [^ <a href=\"[uri [esc &quot;]]\">]quotes[^ </a>]]]");
  Handle("&#42;", ParseHtml, "[[esc &#42;]]");
  Handle("&#x2A;", ParseHtml, "[[esc &#x2A;]]");
  Handle("&#x2a;", ParseHtml, "[[esc &#x2a;]]");
  Handle("&#X2a;", ParseHtml, "[[esc &#X2a;]]");
}
TEST_F(MarkupHandlerTest, HtmlIgnoresBadEscapes) {
  Handle("&", ParseHtml, "[&]");
  Handle("&gt", ParseHtml, "[&gt]");
  Handle("& gt;", ParseHtml, "[& gt;]");
  Handle("&gt ;", ParseHtml, "[&gt ;]");
  Handle("&;", ParseHtml, "[&;]");
  Handle("&<tag>;", ParseHtml, "[&<tag>;]");
  Handle("&<i>;", ParseHtml, "[&[^ <i>];]");
  Handle("&#z;", ParseHtml, "[&#z;]");
  Handle("&#xy;", ParseHtml, "[&#xy;]");
  Handle("&#4", ParseHtml, "[&#4]");
  Handle("&#xa", ParseHtml, "[&#xa]");
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
