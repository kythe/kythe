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

#ifndef KYTHE_CXX_COMMON_SCHEMA_EDGES_H_
#define KYTHE_CXX_COMMON_SCHEMA_EDGES_H_

namespace kythe {
namespace common {
namespace schema {

// Constants for Kythe edges, similar to kythe/go/util/schema/edges/edges.go.

// Edge kind labels
extern const char kChildOf[];
extern const char kExtends[];
extern const char kExtendsPrivate[];
extern const char kExtendsPrivateVirtual[];
extern const char kExtendsProtected[];
extern const char kExtendsProtectedVirtual[];
extern const char kExtendsPublic[];
extern const char kExtendsPublicVirtual[];
extern const char kExtendsVirtual[];
extern const char kGenerates[];
extern const char kNamed[];
extern const char kOverrides[];
extern const char kParam[];
extern const char kSatisfies[];
extern const char kTyped[];

// Edge kinds associated with anchors
extern const char kDefines[];
extern const char kDefinesBinding[];
extern const char kDocuments[];
extern const char kRef[];
extern const char kRefCall[];
extern const char kRefImports[];
extern const char kRefInit[];
extern const char kTagged[];

}  // namespace schema
}  // namespace common
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_SCHEMA_EDGES_H_
