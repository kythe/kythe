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

#ifndef KYTHE_CXX_DOC_HTML_RENDERER_H_
#define KYTHE_CXX_DOC_HTML_RENDERER_H_

#include "kythe/cxx/doc/markup_handler.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/xref.pb.h"

#include <functional>
#include <string>

namespace kythe {

struct HtmlRendererOptions {
  virtual ~HtmlRendererOptions() {}
  /// Used to determine the href attribute value for a link pointing to an
  /// `Anchor`. HtmlRenderer will HTML-escape the link (e.g., ampersands
  /// will be replaced by &amp;).
  std::function<std::string(const proto::Anchor&)> make_link_uri =
      [](const proto::Anchor&) { return ""; };
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
std::vector<std::string> RenderSimpleParams(const proto::MarkedSource& sig);

/// \brief Extract and render the simple identifier for `sig`.
/// \return The empty string if there is no such identifier.
std::string RenderSimpleIdentifier(const proto::MarkedSource& sig);

/// \brief Extract and render the simple qualified name for `sig`.
/// \param include_identifier if set, include the identifier on the qualified
/// name.
/// \return The empty string if there is no such identifier.
std::string RenderSimpleQualifiedName(const proto::MarkedSource& sig,
                                      bool include_identifier);

}  // namespace kythe

#endif
