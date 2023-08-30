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

#include <stack>

#include "kythe/cxx/doc/markup_handler.h"

namespace kythe {
namespace {
/// Don't recurse more than this many times when rendering MarkedSource.
constexpr size_t kMaxRenderDepth = 10;

/// \brief A RAII class to deal with styled div/span tags.
class CssTag {
 public:
  enum class Kind { Div, Span, Pre };
  /// \param kind the kind of tag to open.
  /// \param style the CSS style to apply to the tag. Must be escaped.
  /// \param buffer must outlive CssTag.
  CssTag(Kind kind, const std::string& style, std::string* buffer)
      : kind_(kind), buffer_(buffer) {
    OpenTag(kind, style, buffer);
  }
  ~CssTag() { CloseTag(kind_, buffer_); }
  static void OpenTag(Kind kind, const std::string& style,
                      std::string* buffer) {
    buffer->append("<");
    buffer->append(label(kind));
    buffer->append(" class=\"");
    buffer->append(style);
    buffer->append("\">");
  }
  static void CloseTag(Kind kind, std::string* buffer) {
    buffer->append("</");
    buffer->append(label(kind));
    buffer->append(">");
  }
  static const char* label(Kind kind) {
    return kind == Kind::Div ? "div" : (kind == Kind::Span ? "span" : "pre");
  }
  CssTag(const CssTag& o) = delete;

 private:
  Kind kind_;
  std::string* buffer_;
};

std::string RenderPrintable(const HtmlRendererOptions& options,
                            const std::vector<MarkupHandler>& handlers,
                            const proto::Printable& printable_proto) {
  Printable printable(printable_proto);
  auto markdoc = HandleMarkup(handlers, printable);
  return RenderHtml(options, markdoc);
}

/// \brief Appends a representation of `c` to `buffer`, possibly using an HTML
/// entity instead of a literal character.
void AppendEscapedHtmlCharacter(std::string* buffer, char c) {
  switch (c) {
    case '<':
      buffer->append("&lt;");
      break;
    case '>':
      buffer->append("&gt;");
      break;
    case '&':
      buffer->append("&amp;");
      break;
    default:
      buffer->push_back(c);
  }
}

/// \brief Content of a tagged block (e.g., a @param or a @returns).
struct TaggedText {
  std::string buffer;
};

/// \brief A map from tag block IDs and ordinals to their content.
using TagBlocks =
    std::map<std::pair<PrintableSpan::TagBlockId, size_t>, TaggedText>;

/// \brief Renders the content of `tag_blocks` to the block `out`.
void RenderTagBlocks(const HtmlRendererOptions& options,
                     const TagBlocks& tag_blocks, TaggedText* out) {
  bool first_block = true;
  PrintableSpan::TagBlockId block_id;
  for (const auto& block : tag_blocks) {
    if (first_block || block_id != block.first.first) {
      if (!first_block) {
        out->buffer.append("</ul>");
        CssTag::CloseTag(CssTag::Kind::Div, &out->buffer);
      }
      block_id = block.first.first;
      first_block = false;
      {
        CssTag title(CssTag::Kind::Div, options.tag_section_title_div,
                     &out->buffer);
        switch (block.first.first) {
          case PrintableSpan::TagBlockId::Author:
            out->buffer.append("Author");
            break;
          case PrintableSpan::TagBlockId::Returns:
            out->buffer.append("Returns");
            break;
          case PrintableSpan::TagBlockId::Since:
            out->buffer.append("Since");
            break;
          case PrintableSpan::TagBlockId::Version:
            out->buffer.append("Version");
            break;
          case PrintableSpan::TagBlockId::Throws:
            out->buffer.append("Throws");
            break;
          case PrintableSpan::TagBlockId::Param:
            out->buffer.append("Parameter");
            break;
          case PrintableSpan::TagBlockId::See:
            out->buffer.append("See");
            break;
        }
      }
      CssTag::OpenTag(CssTag::Kind::Div, options.tag_section_content_div,
                      &out->buffer);
      out->buffer.append("<ul>");
    }
    out->buffer.append("<li>");
    out->buffer.append(block.second.buffer);
    out->buffer.append("</li>");
  }
  if (!first_block) {
    // We've opened a ul and a div that we need to close.
    out->buffer.append("</ul>");
    CssTag::CloseTag(CssTag::Kind::Div, &out->buffer);
  }
}

const char* TagNameForStyle(PrintableSpan::Style style) {
  switch (style) {
    case PrintableSpan::Style::Bold:
      return "b";
    case PrintableSpan::Style::Italic:
      return "i";
    case PrintableSpan::Style::H1:
      return "h1";
    case PrintableSpan::Style::H2:
      return "h2";
    case PrintableSpan::Style::H3:
      return "h3";
    case PrintableSpan::Style::H4:
      return "h4";
    case PrintableSpan::Style::H5:
      return "h5";
    case PrintableSpan::Style::H6:
      return "h6";
    case PrintableSpan::Style::Big:
      return "big";
    case PrintableSpan::Style::Small:
      return "small";
    case PrintableSpan::Style::Blockquote:
      return "blockquote";
    case PrintableSpan::Style::Superscript:
      return "sup";
    case PrintableSpan::Style::Subscript:
      return "sub";
    case PrintableSpan::Style::Underline:
      return "ul";
  }
}

template <typename SourceString>
void AppendEscapedHtmlString(const SourceString& source, std::string* dest) {
  dest->reserve(dest->size() + source.size());
  for (char c : source) {
    AppendEscapedHtmlCharacter(dest, c);
  }
}

/// Target buffer for RenderSimpleIdentifier.
class RenderSimpleIdentifierTarget {
 public:
  /// \brief Escapes and appends `source` to the buffer.
  template <typename SourceString>
  void Append(const SourceString& source) {
    if (!prepend_buffer_.empty() && !source.empty()) {
      AppendEscapedHtmlString(prepend_buffer_, &buffer_);
      prepend_buffer_.clear();
    }
    AppendEscapedHtmlString(source, &buffer_);
  }
  /// \brief Escapes and adds `source` before the (non-empty) text that would
  /// be added by the next call to `Append`.
  template <typename SourceString>
  void AppendFinalListToken(const SourceString& source) {
    prepend_buffer_.append(std::string(source));
  }
  const std::string buffer() const { return buffer_; }
  void AppendRaw(const std::string& text) { buffer_.append(text); }
  /// \brief Make sure that there's a space between the current content of the
  /// buffer and whatever is appended to it later on.
  void AppendHeuristicSpace() {
    if (!buffer_.empty() && buffer_[buffer_.size() - 1] != ' ') {
      prepend_buffer_.push_back(' ');
    }
  }

 private:
  /// The buffer used to hold escaped data.
  std::string buffer_;
  /// Unescaped text that should be escaped and appended before any other text
  /// is appended to `buffer_`.
  std::string prepend_buffer_;
};

/// State while recursing through MarkedSource for RenderSimpleIdentifier.
struct RenderSimpleIdentifierState {
  const HtmlRendererOptions* options = nullptr;
  bool render_identifier = false;
  bool render_context = false;
  bool render_types = false;
  bool render_parameters = false;
  bool render_initializer = false;
  bool render_modifier = false;
  bool in_identifier = false;
  bool in_context = false;
  bool in_parameter = false;
  bool in_type = false;
  bool in_initializer = false;
  bool in_modifier = false;
  bool linkify = false;
  std::string base_ticket;
  std::string get_link(const proto::common::MarkedSource& sig) {
    if (options == nullptr || !linkify) {
      return "";
    }
    std::string link;
    auto try_link = [&](const std::string& ticket) {
      if (const auto* node_info = options->node_info(ticket)) {
        if (const auto* anchor =
                options->anchor_for_ticket(node_info->definition())) {
          link = options->make_semantic_link_uri(*anchor, ticket);
          if (link.empty()) {
            link = options->make_link_uri(*anchor);
          }
        }
      }
    };
    for (const auto& plink : sig.link()) {
      for (const auto& ptick : plink.definition()) {
        try_link(ptick);
      }
    }
    if (link.empty() && should_infer_link()) {
      try_link(base_ticket);
    }
    return link;
  }
  bool should_infer_link() const {
    return (in_identifier && !base_ticket.empty() && !in_context &&
            !in_parameter && !in_type && !in_initializer && !in_modifier);
  }
  bool should_render(const proto::common::MarkedSource& node) const {
    return (render_context && in_context) ||
           (render_identifier && in_identifier) ||
           (render_parameters && in_parameter) || (render_types && in_type) ||
           (render_initializer && in_initializer) ||
           (render_modifier && in_modifier);
  }
  bool will_render(const proto::common::MarkedSource& child, size_t depth) {
    if (depth >= kMaxRenderDepth) return false;
    switch (child.kind()) {
      case proto::common::MarkedSource::IDENTIFIER:
        return true;
      case proto::common::MarkedSource::BOX:
        return true;
      case proto::common::MarkedSource::PARAMETER:
        return render_parameters;
      case proto::common::MarkedSource::TYPE:
        return render_types;
      case proto::common::MarkedSource::CONTEXT:
        return render_context;
      case proto::common::MarkedSource::INITIALIZER:
        return render_initializer;
      case proto::common::MarkedSource::MODIFIER:
        return render_modifier;
      default:
        return false;
    }
  }
};

void RenderSimpleIdentifier(const proto::common::MarkedSource& sig,
                            RenderSimpleIdentifierTarget* out,
                            RenderSimpleIdentifierState state, size_t depth) {
  if (depth >= kMaxRenderDepth) {
    return;
  }
  switch (sig.kind()) {
    case proto::common::MarkedSource::IDENTIFIER:
      state.in_identifier = true;
      break;
    case proto::common::MarkedSource::PARAMETER:
      if (!state.render_parameters) {
        return;
      }
      state.in_parameter = true;
      break;
    case proto::common::MarkedSource::TYPE:
      if (!state.render_types) {
        return;
      }
      state.in_type = true;
      break;
    case proto::common::MarkedSource::CONTEXT:
      if (!state.render_context) {
        return;
      }
      state.in_context = true;
      break;
    case proto::common::MarkedSource::INITIALIZER:
      if (!state.render_initializer) {
        return;
      }
      state.in_initializer = true;
      break;
    case proto::common::MarkedSource::BOX:
      break;
    case proto::common::MarkedSource::MODIFIER:
      if (!state.render_modifier) {
        return;
      }
      state.in_modifier = true;
      break;
    default:
      return;
  }
  bool has_open_link = false;
  if (state.should_render(sig)) {
    std::string link_text = state.get_link(sig);
    if (!link_text.empty()) {
      out->AppendRaw("<a href=\"");
      out->AppendRaw(link_text);
      out->AppendRaw("\" title=\"");
      {
        RenderSimpleIdentifierTarget target;
        RenderSimpleIdentifierState state;
        state.render_identifier = true;
        state.render_context = true;
        state.render_types = true;
        RenderSimpleIdentifier(sig, &target, state, 0);
        out->AppendRaw(target.buffer());
      }
      out->AppendRaw("\">");
      has_open_link = true;
    }
    out->Append(sig.pre_text());
  }
  int last_rendered_child = -1;
  for (int child = 0; child < sig.child_size(); ++child) {
    if (state.will_render(sig.child(child), depth + 1)) {
      last_rendered_child = child;
    }
  }
  for (int child = 0; child < sig.child_size(); ++child) {
    if (state.will_render(sig.child(child), depth + 1)) {
      RenderSimpleIdentifier(sig.child(child), out, state, depth + 1);
      if (state.should_render(sig)) {
        if (last_rendered_child > child) {
          out->Append(sig.post_child_text());
        } else if (sig.add_final_list_token()) {
          out->AppendFinalListToken(sig.post_child_text());
        }
      }
    }
  }
  if (state.should_render(sig)) {
    out->Append(sig.post_text());
    if (has_open_link) {
      out->AppendRaw("</a>");
    }
    if (sig.kind() == proto::common::MarkedSource::TYPE) {
      out->AppendHeuristicSpace();
    }
  }
}

/// Render identifiers underneath PARAMETER nodes with no other non-BOXes in
/// between.
void RenderSimpleParams(const proto::common::MarkedSource& sig,
                        std::vector<std::string>* out, size_t depth) {
  if (depth >= kMaxRenderDepth) {
    return;
  }
  switch (sig.kind()) {
    case proto::common::MarkedSource::BOX:
      for (const auto& child : sig.child()) {
        RenderSimpleParams(child, out, depth + 1);
      }
      break;
    case proto::common::MarkedSource::PARAMETER:
      for (const auto& child : sig.child()) {
        out->emplace_back();
        RenderSimpleIdentifierTarget target;
        RenderSimpleIdentifierState state;
        state.render_identifier = true;
        RenderSimpleIdentifier(child, &target, state, depth + 1);
        out->back().append(target.buffer());
      }
      break;
    default:
      break;
  }
}
}  // anonymous namespace

const proto::common::NodeInfo* DocumentHtmlRendererOptions::node_info(
    const std::string& ticket) const {
  auto node = document_.nodes().find(ticket);
  return node == document_.nodes().end() ? nullptr : &node->second;
}

const proto::Anchor* DocumentHtmlRendererOptions::anchor_for_ticket(
    const std::string& ticket) const {
  auto anchor = document_.definition_locations().find(ticket);
  return anchor == document_.definition_locations().end() ? nullptr
                                                          : &anchor->second;
}

std::string RenderSignature(const HtmlRendererOptions& options,
                            const proto::common::MarkedSource& sig,
                            bool linkify, const std::string& base_ticket) {
  RenderSimpleIdentifierTarget target;
  RenderSimpleIdentifierState state;
  state.render_identifier = true;
  state.render_types = true;
  state.render_parameters = true;
  state.render_modifier = true;
  state.linkify = linkify;
  state.options = &options;
  state.base_ticket = base_ticket;
  RenderSimpleIdentifier(sig, &target, state, 0);
  return target.buffer();
}

std::string RenderSimpleIdentifier(const proto::common::MarkedSource& sig) {
  RenderSimpleIdentifierTarget target;
  RenderSimpleIdentifierState state;
  state.render_identifier = true;
  RenderSimpleIdentifier(sig, &target, state, 0);
  return target.buffer();
}

std::string RenderSimpleQualifiedName(const proto::common::MarkedSource& sig,
                                      bool include_identifier) {
  RenderSimpleIdentifierTarget target;
  RenderSimpleIdentifierState state;
  state.render_identifier = include_identifier;
  state.render_context = true;
  RenderSimpleIdentifier(sig, &target, state, 0);
  return target.buffer();
}

std::string RenderInitializer(const proto::common::MarkedSource& sig) {
  RenderSimpleIdentifierTarget target;
  RenderSimpleIdentifierState state;
  state.render_initializer = true;
  RenderSimpleIdentifier(sig, &target, state, 0);
  return target.buffer();
}

std::vector<std::string> RenderSimpleParams(
    const proto::common::MarkedSource& sig) {
  std::vector<std::string> result;
  RenderSimpleParams(sig, &result, 0);
  return result;
}

std::string RenderHtml(const HtmlRendererOptions& options,
                       const Printable& printable) {
  struct OpenSpan {
    const PrintableSpan* span;
    bool valid;
  };
  struct FormatState {
    bool in_pre_block;
  };
  std::stack<OpenSpan> open_spans;
  // To avoid entering multiple <pre> blocks, we keep track of whether we're
  // currently in a <pre> context. This does not affect escaping, since
  // tags can appear in a <pre>.
  std::stack<FormatState> format_states;
  // Elements on `open_tags` point to values of `tag_blocks`. The element on
  // top of the stack is the tag block whose buffer we're currently appending
  // data to (if any). This stack should usually have one or zero elements,
  // given the syntactic restrictions of the markup languages we're translating
  // from.
  std::stack<TaggedText*> open_tags;
  std::map<std::pair<PrintableSpan::TagBlockId, size_t>, TaggedText> tag_blocks;
  TaggedText main_text;
  // `out` points to either `main_text` if `open_tags` is empty or a value of
  // `tag_blocks` (particularly, the one referenced by the top of `open_tags`)
  // if the stack is non-empty.
  TaggedText* out = &main_text;
  PrintableSpan default_span(0, printable.text().size(),
                             PrintableSpan::Semantic::Raw);
  open_spans.push(OpenSpan{&default_span, true});
  format_states.push(FormatState{false});
  size_t current_span = 0;
  for (size_t i = 0; i <= printable.text().size(); ++i) {
    // Normalized PrintableSpans have all empty or negative-length spans
    // dropped.
    while (!open_spans.empty() && open_spans.top().span->end() == i) {
      switch (open_spans.top().span->semantic()) {
        case PrintableSpan::Semantic::TagBlock: {
          if (!open_tags.empty()) {
            open_tags.pop();
          }
          out = open_tags.empty() ? &main_text : open_tags.top();
        } break;
        case PrintableSpan::Semantic::UriLink:
          out->buffer.append("</a>");
          break;
        case PrintableSpan::Semantic::Uri:
          out->buffer.append("\">");
          break;
        case PrintableSpan::Semantic::Link:
          if (open_spans.top().valid) {
            out->buffer.append("</a>");
          }
          break;
        case PrintableSpan::Semantic::CodeRef:
          out->buffer.append("</tt>");
          break;
        case PrintableSpan::Semantic::Paragraph:
          out->buffer.append("</p>");
          break;
        case PrintableSpan::Semantic::ListItem:
          out->buffer.append("</li>");
          break;
        case PrintableSpan::Semantic::UnorderedList:
          out->buffer.append("</ul>");
          break;
        case PrintableSpan::Semantic::Styled:
          out->buffer.append("</");
          out->buffer.append(TagNameForStyle(open_spans.top().span->style()));
          out->buffer.append(">");
          break;
        case PrintableSpan::Semantic::CodeBlock:
          if (!format_states.empty()) {
            format_states.pop();
            if (!format_states.empty() && !format_states.top().in_pre_block) {
              out->buffer.append("</pre>");
            }
          }
          break;
        default:
          break;
      }
      open_spans.pop();
    }
    if (open_spans.empty() || i == printable.text().size()) {
      // default_span is first to enter and last to leave; there also may
      // be no empty spans.
      break;
    }
    while (current_span < printable.spans().size() &&
           printable.spans().span(current_span).begin() == i) {
      open_spans.push({&printable.spans().span(current_span), true});
      ++current_span;
      switch (open_spans.top().span->semantic()) {
        case PrintableSpan::Semantic::TagBlock: {
          auto block = open_spans.top().span->tag_block();
          out = &tag_blocks[block];
          open_tags.push(out);
        } break;
        case PrintableSpan::Semantic::UriLink:
          out->buffer.append("<a ");
          break;
        case PrintableSpan::Semantic::Uri:
          out->buffer.append("href=\"");
          break;
        case PrintableSpan::Semantic::Link:
          open_spans.top().valid = false;  // Invalid until proven otherwise.
          if (open_spans.top().span->link().definition_size() != 0) {
            const auto& definition =
                open_spans.top().span->link().definition(0);
            if (const auto* def_info = options.node_info(definition)) {
              if (!def_info->definition().empty()) {
                if (const auto* def_anchor =
                        options.anchor_for_ticket(def_info->definition())) {
                  open_spans.top().valid = true;
                  out->buffer.append("<a href=\"");
                  auto link_uri =
                      options.make_semantic_link_uri(*def_anchor, definition);
                  if (link_uri.empty()) {
                    link_uri = options.make_link_uri(*def_anchor);
                  }
                  // + 2 for the closing ">.
                  out->buffer.reserve(out->buffer.size() + link_uri.size() + 2);
                  for (auto c : link_uri) {
                    AppendEscapedHtmlCharacter(&out->buffer, c);
                  }
                  out->buffer.append("\">");
                }
              }
            }
          }
          break;
        case PrintableSpan::Semantic::CodeRef:
          out->buffer.append("<tt>");
          break;
        case PrintableSpan::Semantic::Paragraph:
          out->buffer.append("<p>");
          break;
        case PrintableSpan::Semantic::ListItem:
          out->buffer.append("<li>");
          break;
        case PrintableSpan::Semantic::UnorderedList:
          out->buffer.append("<ul>");
          break;
        case PrintableSpan::Semantic::Styled:
          out->buffer.append("<");
          out->buffer.append(TagNameForStyle(open_spans.top().span->style()));
          out->buffer.append(">");
          break;
        case PrintableSpan::Semantic::CodeBlock:
          if (!format_states.empty() && !format_states.top().in_pre_block) {
            out->buffer.append("<pre>");
          }
          format_states.push(FormatState{true});
          break;
        default:
          break;
      }
    }
    if (open_spans.top().span->semantic() == PrintableSpan::Semantic::Escaped) {
      out->buffer.push_back(printable.text()[i]);
    } else if (open_spans.top().span->semantic() !=
               PrintableSpan::Semantic::Markup) {
      char c = printable.text()[i];
      AppendEscapedHtmlCharacter(&out->buffer, c);
    }
  }
  RenderTagBlocks(options, tag_blocks, out);
  return out->buffer;
}

std::string RenderDocument(
    const HtmlRendererOptions& options,
    const std::vector<MarkupHandler>& handlers,
    const proto::DocumentationReply::Document& document) {
  std::string text_out;
  {
    CssTag root(CssTag::Kind::Div, options.doc_div, &text_out);
    {
      CssTag sig(CssTag::Kind::Div, options.signature_div, &text_out);
      {
        CssTag type_div(CssTag::Kind::Div, options.type_name_div, &text_out);
        CssTag type_name(CssTag::Kind::Span, options.name_span, &text_out);
        text_out.append(RenderSignature(options, document.marked_source(), true,
                                        document.ticket()));
      }
      text_out.append(options.make_post_signature_markup(
          document.ticket(), document.marked_source()));
      {
        CssTag detail_div(CssTag::Kind::Div, options.sig_detail_div, &text_out);
        auto kind_name = options.kind_name(document.ticket());
        if (!kind_name.empty()) {
          AppendEscapedHtmlString(kind_name, &text_out);
          text_out.append(" ");
        }
        auto declared_context =
            RenderSimpleQualifiedName(document.marked_source(), false);
        if (declared_context.empty()) {
          if (const auto* node_info = options.node_info(document.ticket())) {
            if (const auto* anchor =
                    options.anchor_for_ticket(node_info->definition())) {
              auto anchor_text = options.format_location(*anchor);
              if (!anchor_text.empty()) {
                text_out.append("declared in ");
                auto anchor_link =
                    options.make_semantic_link_uri(*anchor, document.ticket());
                if (!anchor_link.empty()) {
                  text_out.append("<a href=\"");
                  AppendEscapedHtmlString(anchor_link, &text_out);
                  text_out.append("\">");
                }
                AppendEscapedHtmlString(anchor_text, &text_out);
                if (!anchor_link.empty()) {
                  text_out.append("</a>");
                }
              }
            }
          }
        } else {
          text_out.append("declared by ");
          text_out.append(declared_context);
        }
      }
    }
    auto initializer = RenderInitializer(document.marked_source());
    if (!initializer.empty()) {
      CssTag init_div(CssTag::Kind::Div, options.initializer_div, &text_out);
      text_out.append("Initializer: ");
      bool multiline = initializer.find("\n") != decltype(initializer)::npos;
      CssTag code_div(CssTag::Kind::Pre,
                      multiline ? options.initializer_multiline_pre
                                : options.initializer_pre,
                      &text_out);
      text_out.append(initializer);
    }
    {
      CssTag content_div(CssTag::Kind::Div, options.content_div, &text_out);
      text_out.append(RenderPrintable(options, handlers, document.text()));
    }
    for (const auto& child : document.children()) {
      text_out.append(RenderDocument(options, handlers, child));
    }
  }
  return text_out;
}

}  // namespace kythe
