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
#include "kythe/proto/xref.pb.h"

#include <functional>
#include <string>

namespace kythe {

struct HtmlRendererOptions {
  /// Used to determine the href attribute value for a link pointing to an
  /// `Anchor`. HtmlRenderer will HTML-escape the link (e.g., ampersands
  /// will be replaced by &amp;).
  std::function<std::string(const proto::Anchor&)> make_link_uri =
      [](const proto::Anchor&) { return ""; };
};

/// \brief Render `document` as HTML according to `options`.
std::string RenderHtml(const HtmlRendererOptions& options,
                       const Printable& printable);

}  // namespace kythe

#endif
