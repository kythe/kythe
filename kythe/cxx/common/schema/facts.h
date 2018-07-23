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

#ifndef KYTHE_CXX_COMMON_SCHEMA_FACTS_H_
#define KYTHE_CXX_COMMON_SCHEMA_FACTS_H_

namespace kythe {
namespace common {
namespace schema {

// Constants for Kythe facts, similar to kythe/go/util/schema/facts/facts.go.

extern const char kFactAnchorEnd[];
extern const char kFactAnchorStart[];
extern const char kFactCode[];
extern const char kFactComplete[];
extern const char kFactContextURL[];
extern const char kFactDetails[];
extern const char kFactDocURI[];
extern const char kFactMessage[];
extern const char kFactNodeKind[];
extern const char kFactParamDefault[];
extern const char kFactSnippetEnd[];
extern const char kFactSnippetStart[];
extern const char kFactSubkind[];
extern const char kFactText[];
extern const char kFactTextEncoding[];

}  // namespace schema
}  // namespace common
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_SCHEMA_FACTS_H_
