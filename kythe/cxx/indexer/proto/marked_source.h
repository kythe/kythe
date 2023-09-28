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
#include "kythe/cxx/indexer/proto/proto_graph_builder.h"

namespace kythe {

// `name` is a proto-style dotted qualified name
// `root` is the node to turn into a (box (context) (identifier)) or an
// identifier, depending on the structure of `name`.
// If given a vname, it will be added as a definition for the identifier.
// Returns true if at least one token was found.
bool GenerateMarkedSourceForDottedName(
    absl::string_view name, MarkedSource* root,
    std::optional<proto::VName> vname = std::nullopt);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::Descriptor* descriptor, ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumDescriptor* descriptor,
    ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumValueDescriptor* descriptor,
    ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::FieldDescriptor* descriptor,
    ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::ServiceDescriptor* descriptor,
    ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::MethodDescriptor* descriptor,
    ProtoGraphBuilder* builder);

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::OneofDescriptor* descriptor,
    ProtoGraphBuilder* builder);

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_MARKED_SOURCE_H_
