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

#ifndef KYTHE_CXX_DOC_JAVADOXYGEN_MARKUP_HANDLER_H_
#define KYTHE_CXX_DOC_JAVADOXYGEN_MARKUP_HANDLER_H_

#include "kythe/cxx/doc/markup_handler.h"
#include "kythe/proto/xref.pb.h"

namespace kythe {

/// \brief Parse a Printable containing Javadoc and/or Doxygen style directives.
/// \note This does not handle parsing Markdown.
void ParseJavadoxygen(const Printable& in_message, PrintableSpans* out_spans);

}  // namespace kythe

#endif
