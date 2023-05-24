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

#include "kythe/cxx/common/schema/edges.h"

namespace kythe {
namespace common {
namespace schema {

// Constants for Kythe edges, similar to kythe/go/util/schema/edges/edges.go.

// Edge kind labels
const char kChildOf[] = "/kythe/edge/childof";
const char kExtends[] = "/kythe/edge/extends";
const char kExtendsPrivate[] = "/kythe/edge/extends/private";
const char kExtendsPrivateVirtual[] = "/kythe/edge/extends/private/virtual";
const char kExtendsProtected[] = "/kythe/edge/extends/protected";
const char kExtendsProtectedVirtual[] = "/kythe/edge/extends/protected/virtual";
const char kExtendsPublic[] = "/kythe/edge/extends/public";
const char kExtendsPublicVirtual[] = "/kythe/edge/extends/public/virtual";
const char kExtendsVirtual[] = "/kythe/edge/extends/virtual";
const char kGenerates[] = "/kythe/edge/generates";
const char kNamed[] = "/kythe/edge/named";
const char kOverrides[] = "/kythe/edge/overrides";
const char kParam[] = "/kythe/edge/param";
const char kSatisfies[] = "/kythe/edge/satisfies";
const char kTyped[] = "/kythe/edge/typed";

// Edge kinds associated with anchors
const char kDefines[] = "/kythe/edge/defines";
const char kDefinesBinding[] = "/kythe/edge/defines/binding";
const char kDocuments[] = "/kythe/edge/documents";
const char kRef[] = "/kythe/edge/ref";
const char kRefCall[] = "/kythe/edge/ref/call";
const char kRefImports[] = "/kythe/edge/ref/imports";
const char kRefInit[] = "/kythe/edge/ref/init";
const char kTagged[] = "/kythe/edge/tagged";

}  // namespace schema
}  // namespace common
}  // namespace kythe
