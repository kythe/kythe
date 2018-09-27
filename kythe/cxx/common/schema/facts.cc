/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/schema/facts.h"

namespace kythe {
namespace common {
namespace schema {

const char kFactAnchorEnd[] = "/kythe/loc/end";
const char kFactAnchorStart[] = "/kythe/loc/start";
const char kFactCode[] = "/kythe/code";
const char kFactComplete[] = "/kythe/complete";
const char kFactContextURL[] = "/kythe/context/url";
const char kFactDetails[] = "/kythe/details";
const char kFactDocURI[] = "/kythe/doc/uri";
const char kFactMessage[] = "/kythe/message";
const char kFactNodeKind[] = "/kythe/node/kind";
const char kFactParamDefault[] = "/kythe/param/default";
const char kFactSnippetEnd[] = "/kythe/snippet/end";
const char kFactSnippetStart[] = "/kythe/snippet/start";
const char kFactSubkind[] = "/kythe/subkind";
const char kFactText[] = "/kythe/text";
const char kFactTextEncoding[] = "/kythe/text/encoding";

}  // namespace schema
}  // namespace common
}  // namespace kythe
