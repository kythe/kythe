/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_PROTO_MARKED_SOURCE_H_
#define KYTHE_CXX_INDEXER_PROTO_MARKED_SOURCE_H_

#include <optional>

#include "google/protobuf/descriptor.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"

namespace kythe {

// `name` is a proto-style dotted qualified name
// `root` is the node to turn into a (box (context) (identifier)) or an
// identifier, depending on the structure of `name`.
// Returns true if at least one token was found.
bool GenerateMarkedSourceForDottedName(absl::string_view name,
                                       MarkedSource* root);

// Given a proto descriptor, generates an appropriate code fact. Returns
// `None` if a code fact couldn't be generated.
template <typename T>
std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const T* descriptor) {
  MarkedSource ms;
  if (GenerateMarkedSourceForDottedName(descriptor->full_name(), &ms)) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::Descriptor* descriptor);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumDescriptor* descriptor);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumValueDescriptor* descriptor);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::FieldDescriptor* descriptor);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::ServiceDescriptor* descriptor);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::MethodDescriptor* descriptor);

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_MARKED_SOURCE_H_
