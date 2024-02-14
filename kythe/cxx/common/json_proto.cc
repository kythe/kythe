/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/json_proto.h"

#include <memory>
#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/any.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message.h"
#include "google/protobuf/type.pb.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/util/type_resolver.h"
#include "google/protobuf/util/type_resolver_util.h"

namespace kythe {
namespace {
using ::google::protobuf::DescriptorPool;
using ::google::protobuf::util::JsonParseOptions;
using ::google::protobuf::util::TypeResolver;

class PermissiveTypeResolver : public TypeResolver {
 public:
  explicit PermissiveTypeResolver(const DescriptorPool* pool)
      : impl_(google::protobuf::util::NewTypeResolverForDescriptorPool("",
                                                                       pool)) {}

  absl::Status ResolveMessageType(
      const std::string& type_url,
      google::protobuf::Type* message_type) override {
    absl::string_view adjusted = type_url;
    adjusted.remove_prefix(type_url.rfind('/') + 1);
    return impl_->ResolveMessageType(absl::StrCat("/", adjusted), message_type);
  }

  absl::Status ResolveEnumType(const std::string& type_url,
                               google::protobuf::Enum* enum_type) override {
    absl::string_view adjusted = type_url;
    adjusted.remove_prefix(type_url.rfind('/') + 1);
    return impl_->ResolveEnumType(absl::StrCat("/", adjusted), enum_type);
  }

 private:
  std::unique_ptr<TypeResolver> impl_;
};

TypeResolver* GetGeneratedTypeResolver() {
  static TypeResolver* generated_resolver =
      new PermissiveTypeResolver(DescriptorPool::generated_pool());
  return generated_resolver;
}

struct MaybeDeleteResolver {
  void operator()(TypeResolver* resolver) const {
    if (resolver != GetGeneratedTypeResolver()) {
      delete resolver;
    }
  }
};

std::unique_ptr<TypeResolver, MaybeDeleteResolver> MakeTypeResolverForPool(
    const DescriptorPool* pool) {
  if (pool == DescriptorPool::generated_pool()) {
    return std::unique_ptr<TypeResolver, MaybeDeleteResolver>(
        GetGeneratedTypeResolver());
  }
  return std::unique_ptr<TypeResolver, MaybeDeleteResolver>(
      new PermissiveTypeResolver(pool));
}

absl::Status WriteMessageAsJsonToStringInternal(
    const google::protobuf::Message& message, std::string* out) {
  auto resolver =
      MakeTypeResolverForPool(message.GetDescriptor()->file()->pool());

  google::protobuf::util::JsonPrintOptions options;
  options.preserve_proto_field_names = true;

  auto status = google::protobuf::util::BinaryToJsonString(
      resolver.get(), message.GetDescriptor()->full_name(),
      message.SerializeAsString(), out, options);
  if (!status.ok()) {
    return absl::Status(static_cast<absl::StatusCode>(status.code()),
                        std::string(status.message()));
  }
  return absl::OkStatus();
}

JsonParseOptions DefaultParseOptions() {
  JsonParseOptions options;
  options.case_insensitive_enum_parsing = false;
  return options;
}

}  // namespace

bool WriteMessageAsJsonToString(const google::protobuf::Message& message,
                                std::string* out) {
  auto status = WriteMessageAsJsonToStringInternal(message, out);
  if (!status.ok()) {
    LOG(ERROR) << status.ToString();
  }
  return status.ok();
}

absl::StatusOr<std::string> WriteMessageAsJsonToString(
    const google::protobuf::Message& message) {
  std::string result;
  auto status = WriteMessageAsJsonToStringInternal(message, &result);
  if (!status.ok()) {
    return status;
  }
  return result;
}

absl::Status ParseFromJsonStream(
    google::protobuf::io::ZeroCopyInputStream* input,
    const JsonParseOptions& options, google::protobuf::Message* message) {
  auto resolver =
      MakeTypeResolverForPool(message->GetDescriptor()->file()->pool());

  std::string binary;
  google::protobuf::io::StringOutputStream output(&binary);
  auto status = google::protobuf::util::JsonToBinaryStream(
      resolver.get(), message->GetDescriptor()->full_name(), input, &output,
      options);

  if (!status.ok()) {
    return absl::Status(static_cast<absl::StatusCode>(status.code()),
                        std::string(status.message()));
  }
  if (!message->ParseFromString(binary)) {
    return absl::InvalidArgumentError(
        "JSON transcoder produced invalid protobuf output.");
  }
  return absl::OkStatus();
}

absl::Status ParseFromJsonStream(
    google::protobuf::io::ZeroCopyInputStream* input,
    google::protobuf::Message* message) {
  return ParseFromJsonStream(input, DefaultParseOptions(), message);
}

absl::Status ParseFromJsonString(absl::string_view input,
                                 const JsonParseOptions& options,
                                 google::protobuf::Message* message) {
  google::protobuf::io::ArrayInputStream stream(input.data(), input.size());
  return ParseFromJsonStream(&stream, options, message);
}

absl::Status ParseFromJsonString(absl::string_view input,
                                 google::protobuf::Message* message) {
  return ParseFromJsonString(input, DefaultParseOptions(), message);
}

void PackAny(const google::protobuf::Message& message,
             absl::string_view type_uri, google::protobuf::Any* out) {
  out->set_type_url(type_uri.data(), type_uri.size());
  message.SerializeToString(out->mutable_value());
}

bool UnpackAny(const google::protobuf::Any& any,
               google::protobuf::Message* result) {
  google::protobuf::io::ArrayInputStream stream(any.value().data(),
                                                any.value().size());
  google::protobuf::io::CodedInputStream coded_input_stream(&stream);
  return result->ParseFromCodedStream(&coded_input_stream);
}
}  // namespace kythe
