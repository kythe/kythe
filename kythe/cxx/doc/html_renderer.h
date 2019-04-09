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

#ifndef KYTHE_CXX_DOC_HTML_RENDERER_H_
#define KYTHE_CXX_DOC_HTML_RENDERER_H_

#include <functional>
#include <string>

#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/doc/markup_handler.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/xref.pb.h"

namespace kythe {

struct HtmlRendererOptions {
  virtual ~HtmlRendererOptions() {}
  /// Used to determine the href attribute value for a link pointing to an
  /// `Anchor`. HtmlRenderer will HTML-escape the link (e.g., ampersands
  /// will be replaced by &amp;).
  std::function<std::string(const proto::Anchor&)> make_link_uri =
      [](const proto::Anchor&) { return ""; };
  /// Used to format a location from an `Anchor`. HtmlRenderer will HTML-escape
  /// the resulting text.
  std::function<std::string(const proto::Anchor&)> format_location =
      [](const proto::Anchor& anchor) {
        auto uri = URI::FromString(anchor.ticket());
        return uri.first ? std::string(uri.second.v_name().path()) : "";
      };
  /// Used to determine the href attribute value for a link pointing to an
  /// `Anchor`. HtmlRenderer will HTML-escape the link (e.g., ampersands
  /// will be replaced by &amp;). semantic_ticket is the ticket associated with
  /// the `Anchor`. If this returns the empty string, the renderer will try
  /// `make_link_uri`.
  std::function<std::string(const proto::Anchor&, const std::string&)>
      make_semantic_link_uri =
          [](const proto::Anchor&, const std::string& semantic_ticket) {
            return "";
          };
  /// Used to provide additional markup after the signature for the provided
  /// ticket with the given `MarkedSource`. The result is included directly in
  /// the output without escaping.
  std::function<std::string(const std::string&,
                            const proto::common::MarkedSource&)>
      make_post_signature_markup =
          [](const std::string& semantic_ticket,
             const proto::common::MarkedSource& marked_soure) { return ""; };
  /// Used to retrieve `NodeInfo` for the given semantic ticket.
  virtual const proto::common::NodeInfo* node_info(const std::string&) const {
    return nullptr;
  }
  /// Used to retrieve a string representing the kind of the given semantic
  /// ticket.
  std::function<std::string(const std::string&)> kind_name =
      [](const std::string&) { return ""; };
  /// Used to map from an anchor's ticket to that `Anchor`.
  virtual const proto::Anchor* anchor_for_ticket(const std::string&) const {
    return nullptr;
  }
  /// Configures the CSS class to apply to the outermost div of a document.
  std::string doc_div = "kythe-doc";
  /// Configures the CSS class to apply to a tag's label (e.g, "Authors:")
  std::string tag_section_title_div = "kythe-doc-tag-section-title";
  /// Configures the CSS class to apply to a tag's content.
  std::string tag_section_content_div = "kythe-doc-tag-section-content";
  /// Configures the CSS class to apply to signature divs.
  std::string signature_div = "kythe-doc-element-signature";
  /// Configures the CSS class to apply to content divs.
  std::string content_div = "kythe-doc-content";
  /// Configures the CSS class to apply to type name divs.
  std::string type_name_div = "kythe-doc-type-name";
  /// Configures the CSS class to apply to name spans.
  std::string name_span = "kythe-doc-name-span";
  /// Configures the CSS class to apply to signature detail divs.
  std::string sig_detail_div = "kythe-doc-qualified-name";
  /// Configures the CSS class to apply to initializer section divs.
  std::string initializer_div =
      "kythe-doc-initializer-section kythe-doc-qualified-name";
  /// Configures the CSS class to apply to multiline initializer pres.
  std::string initializer_multiline_pre =
      "kythe-doc-initializer kythe-doc-pre-code "
      "kythe-doc-initializer-multiline";
  /// Configures the CSS class to apply to initializer pres.
  std::string initializer_pre = "kythe-doc-initializer kythe-doc-pre-code";
};

class DocumentHtmlRendererOptions : public HtmlRendererOptions {
 public:
  explicit DocumentHtmlRendererOptions(
      const proto::DocumentationReply& document)
      : document_(document) {}
  const proto::common::NodeInfo* node_info(const std::string&) const override;
  const proto::Anchor* anchor_for_ticket(const std::string&) const override;

 private:
  const proto::DocumentationReply& document_;
};

/// \brief Render `printable` as HTML according to `options`.
std::string RenderHtml(const HtmlRendererOptions& options,
                       const Printable& printable);

/// \brief Render `document` as HTML according to `options`, using `handlers` to
/// process markup.
std::string RenderDocument(const HtmlRendererOptions& options,
                           const std::vector<MarkupHandler>& handlers,
                           const proto::DocumentationReply::Document& document);

/// \brief Extract and render the simple identifiers for parameters in `sig`.
std::vector<std::string> RenderSimpleParams(
    const proto::common::MarkedSource& sig);

/// \brief Extract and render the simple identifier for `sig`.
/// \return The empty string if there is no such identifier.
std::string RenderSimpleIdentifier(const proto::common::MarkedSource& sig);

/// \brief Extract and render the simple qualified name for `sig`.
/// \param include_identifier if set, include the identifier on the qualified
/// name.
/// \return The empty string if there is no such identifier.
std::string RenderSimpleQualifiedName(const proto::common::MarkedSource& sig,
                                      bool include_identifier);

/// \brief Extract and render a plaintext initializer for `sig`.
/// \return The empty string if there is no such initializer.
std::string RenderInitializer(const proto::common::MarkedSource& sig);

/// \brief Render `sig` as a full signature.
/// \param base_ticket if set and `linkify` is true, use this ticket to try
/// and generate links over the bare `IDENTIFIER` nodes in `sig`.
std::string RenderSignature(const HtmlRendererOptions& options,
                            const proto::common::MarkedSource& sig,
                            bool linkify, const std::string& base_ticket);

}  // namespace kythe

#endif
