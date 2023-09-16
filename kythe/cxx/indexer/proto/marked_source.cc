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

#include <functional>
#include <optional>

#include "absl/strings/str_split.h"
#include "google/protobuf/descriptor.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"
#include "kythe/cxx/common/kythe_uri.h"
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
    absl::string_view kind, const T* descriptor) {
  MarkedSource ms;
  auto* mod = ms.add_child();
  mod->set_kind(MarkedSource::MODIFIER);
  mod->set_pre_text(kind);
  mod->set_post_text(" ");
  if (GenerateMarkedSourceForDottedName(descriptor->full_name(),
                                        ms.add_child())) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::Descriptor* descriptor) {
  return GenerateMarkedSourceForDescriptor("message", descriptor);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumDescriptor* descriptor) {
  return GenerateMarkedSourceForDescriptor("enum", descriptor);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::EnumValueDescriptor* descriptor) {
  // EnumValueDescriptor::full_name leaves off the parent enum's name.
  std::string full_name =
      descriptor->type()->full_name() + "." + descriptor->name();
  MarkedSource ms;
  if (GenerateMarkedSourceForDottedName(full_name, &ms)) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForType(
    const google::protobuf::FieldDescriptor* descriptor) {
  MarkedSource type;
  type.set_kind(MarkedSource::TYPE);
  switch (descriptor->type()) {
    case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
      if (descriptor->is_map()) {
        type.set_pre_text("map<");
        type.set_post_child_text(", ");
        type.set_post_text(">");

        auto key =
            GenerateMarkedSourceForType(descriptor->message_type()->map_key());
        auto val = GenerateMarkedSourceForType(
            descriptor->message_type()->map_value());
        if (!key || !val) {
          return std::nullopt;
        }

        *type.add_child() = *key;
        *type.add_child() = *val;
      } else if (!GenerateMarkedSourceForDottedName(
                     descriptor->message_type()->full_name(),
                     type.add_child())) {
        return std::nullopt;
      }
      break;
    case google::protobuf::FieldDescriptor::TYPE_ENUM:
      if (!GenerateMarkedSourceForDottedName(
              descriptor->enum_type()->full_name(), type.add_child())) {
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
    const std::function<proto::VName(const google::protobuf::FieldDescriptor*)>&
        vname_for_desc) {
  std::string full_name;
  if (const google::protobuf::OneofDescriptor* oneof =
          descriptor->containing_oneof()) {
    full_name = oneof->full_name() + "." + descriptor->name();
  } else {
    full_name = descriptor->full_name();
  }
  MarkedSource ms;
  ms.set_post_child_text(" ");
  if (!descriptor->containing_oneof() && !descriptor->is_map() &&
      descriptor->label()) {
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
          GenerateMarkedSourceForType(descriptor)) {
    *ms.add_child() = *t;
  }
  if (GenerateMarkedSourceForDottedName(full_name, ms.add_child(),
                                        vname_for_desc(descriptor))) {
    return ms;
  }
  return std::nullopt;
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::ServiceDescriptor* descriptor) {
  return GenerateMarkedSourceForDescriptor("service", descriptor);
}

std::optional<MarkedSource> GenerateMarkedSourceForDescriptor(
    const google::protobuf::MethodDescriptor* descriptor) {
  return GenerateMarkedSourceForDescriptor("rpc", descriptor);
}

}  // namespace kythe
