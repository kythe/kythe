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

#include "kythe/cxx/indexer/proto/marked_source.h"

#include <optional>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/descriptor.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/indexer/proto/proto_graph_builder.h"
#include "kythe/proto/common.pb.h"

namespace kythe {
bool GenerateMarkedSourceForDottedName(absl::string_view name,
                                       MarkedSource* root,
                                       std::optional<proto::VName> vname) {
  std::vector<absl::string_view> tokens = absl::StrSplit(name, '.');
  if (tokens.empty()) {
    return false;
  }
  if (tokens.size() == 1) {
    root->set_kind(MarkedSource::IDENTIFIER);
    root->set_pre_text(std::string(tokens[0]));
    if (vname) {
      root->add_link()->add_definition(URI(*vname).ToString());
    }
  } else {
    auto* context = root->add_child();
    auto* ident = root->add_child();
    ident->set_kind(MarkedSource::IDENTIFIER);
    ident->set_pre_text(std::string(tokens.back()));
    if (vname) {
      ident->add_link()->add_definition(URI(*vname).ToString());
    }
    tokens.pop_back();
    context->set_kind(MarkedSource::CONTEXT);
    context->set_post_child_text(".");
    context->set_add_final_list_token(true);
    for (const auto& token : tokens) {
      auto* node = context->add_child();
      node->set_kind(MarkedSource::IDENTIFIER);
      node->set_pre_text(std::string(token));
    }
  }
  return true;
}

template <typename T>
static std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    absl::string_view kind, const T* descriptor, ProtoGraphBuilder* builder) {
  MarkedSource ms;
  auto* mod = ms.add_child();
  mod->set_kind(MarkedSource::MODIFIER);
  mod->set_pre_text(kind);
  mod->set_post_text(" ");
  std::optional<proto::VName> vname;
  if (builder) {
    vname = builder->VNameForDescriptor(descriptor);
  }
  if (GenerateMarkedSourceForDottedName(descriptor->full_name(), ms.add_child(),
                                        vname)) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::Descriptor* descriptor,
    ProtoGraphBuilder* builder) {
  return GenerateMarkedSourceForDescriptor("message", descriptor, builder);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  return GenerateMarkedSourceForDescriptor("enum", descriptor, builder);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumValueDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  // EnumValueDescriptor::full_name leaves off the parent enum's name.
  std::string full_name = absl::StrCat(
      descriptor->type()->full_name(), ".", descriptor->name());
  MarkedSource ms;
  if (GenerateMarkedSourceForDottedName(
          full_name, &ms, builder->VNameForDescriptor(descriptor))) {
    return ms;
  }
  return std::nullopt;
}

static std::optional<MarkedSource> GenerateMarkedSourceForType(
    const google::protobuf::FieldDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  MarkedSource type;
  type.set_kind(MarkedSource::TYPE);
  switch (descriptor->type()) {
    case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
      if (descriptor->is_map()) {
        type.set_pre_text("map<");
        type.set_post_child_text(", ");
        type.set_post_text(">");

        auto key = GenerateMarkedSourceForType(
            descriptor->message_type()->map_key(), builder);
        auto val = GenerateMarkedSourceForType(
            descriptor->message_type()->map_value(), builder);
        if (!key || !val) {
          return std::nullopt;
        }

        *type.add_child() = *key;
        *type.add_child() = *val;
      } else if (!GenerateMarkedSourceForDottedName(
                     descriptor->message_type()->full_name(), type.add_child(),
                     builder->VNameForDescriptor(descriptor->message_type()))) {
        return std::nullopt;
      }
      break;
    case google::protobuf::FieldDescriptor::TYPE_ENUM:
      if (!GenerateMarkedSourceForDottedName(
              descriptor->enum_type()->full_name(), type.add_child(),
              builder->VNameForDescriptor(descriptor->enum_type()))) {
        return std::nullopt;
      }
      break;
    default:
      type.set_pre_text(descriptor->type_name());
      break;
  }
  return type;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::FieldDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  std::string full_name;
  if (const google::protobuf::OneofDescriptor* oneof =
          descriptor->real_containing_oneof()) {
    full_name = absl::StrCat(oneof->full_name(), ".", descriptor->name());
  } else {
    full_name = descriptor->full_name();
  }
  MarkedSource ms;
  ms.set_post_child_text(" ");
  if (!descriptor->real_containing_oneof() && !descriptor->is_map() &&
      (descriptor->has_presence() || descriptor->is_repeated())) {
    auto* mod = ms.add_child();
    mod->set_kind(MarkedSource::MODIFIER);
    switch (descriptor->label()) {
      case google::protobuf::FieldDescriptor::Label::LABEL_OPTIONAL:
        mod->set_pre_text("optional");
        break;
      case google::protobuf::FieldDescriptor::Label::LABEL_REQUIRED:
        mod->set_pre_text("required");
        break;
      case google::protobuf::FieldDescriptor::Label::LABEL_REPEATED:
        mod->set_pre_text("repeated");
        break;
    }
  }
  if (const std::optional<MarkedSource> t =
          GenerateMarkedSourceForType(descriptor, builder)) {
    *ms.add_child() = *t;
  }
  if (GenerateMarkedSourceForDottedName(
          full_name, ms.add_child(), builder->VNameForDescriptor(descriptor))) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::ServiceDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  return GenerateMarkedSourceForDescriptor("service", descriptor, builder);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::MethodDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  return GenerateMarkedSourceForDescriptor("rpc", descriptor, builder);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::OneofDescriptor* descriptor,
    ProtoGraphBuilder* builder) {
  MarkedSource ms;
  if (GenerateMarkedSourceForDottedName(
          descriptor->full_name(), &ms,
          builder->VNameForDescriptor(descriptor))) {
    return ms;
  }
  return std::nullopt;
}

}  // namespace kythe
