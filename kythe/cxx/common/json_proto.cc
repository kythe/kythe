/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "json_proto.h"

#include <openssl/base64.h>
#include <openssl/sha.h>

#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace kythe {

bool DecodeBase64(const google::protobuf::string &data,
                  google::protobuf::string *decoded) {
  size_t expected_size;
  if (::EVP_DecodedLength(&expected_size, data.size()) != 1) {
    return false;
  }

  if (expected_size == 0) {
    decoded->clear();
    return true;
  }
  decoded->resize(expected_size);

  size_t written_size;
  uint8_t *output = reinterpret_cast<uint8_t *>(&(*decoded)[0]);
  const uint8_t *input = reinterpret_cast<const uint8_t *>(data.data());
  if (::EVP_DecodeBase64(output, &written_size, decoded->size(), input,
                         data.size()) != 1) {
    return false;
  }
  decoded->resize(written_size);
  return true;
}

google::protobuf::string EncodeBase64(const google::protobuf::string &data) {
  google::protobuf::string encoded;

  size_t expected_size;
  CHECK(::EVP_EncodedLength(&expected_size, data.size()));
  CHECK(expected_size > 0);

  // expected_size includes trailing NULL, resize() does not.
  encoded.resize(expected_size - 1);

  const uint8_t *input = reinterpret_cast<const uint8_t *>(data.data());
  uint8_t *output = reinterpret_cast<uint8_t *>(&encoded[0]);
  const size_t encoded_size = ::EVP_EncodeBlock(output, input, data.size());

  // Shrink-to-fit the actual encoded size.
  encoded.resize(encoded_size);
  return encoded;
}

/// \tparam W A RapidJSON Writer.
template <typename W>
bool JsonOfMessage(const google::protobuf::Message &message, W *writer);

/// \tparam W A RapidJSON Writer.
template <typename W>
bool JsonOfValue(const google::protobuf::FieldDescriptor *field,
                 const google::protobuf::Message &message,
                 const google::protobuf::Reflection *reflection, W *writer) {
  using namespace google::protobuf;
  int count = field->is_repeated() ? reflection->FieldSize(message, field) : 1;
  if (field->is_repeated()) {
    if (count == 0) {
      return true;  // Do not emit anything for empty repeated fields.
    }
    writer->Key(field->name().c_str());
    writer->StartArray();
  } else {
    writer->Key(field->name().c_str());
  }

  for (int i = 0; i < count; ++i) {
    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_STRING: {
        google::protobuf::string scratch;
        const auto &value =
            field->is_repeated()
                ? reflection->GetRepeatedStringReference(message, field, i,
                                                         &scratch)
                : reflection->GetStringReference(message, field, &scratch);
        if (field->type() == FieldDescriptor::TYPE_BYTES) {
          writer->String(EncodeBase64(value).c_str());
        } else {
          writer->String(value.c_str());
        }
      } break;
      case FieldDescriptor::CPPTYPE_BOOL: {
        writer->Bool(field->is_repeated()
                         ? reflection->GetRepeatedBool(message, field, i)
                         : reflection->GetBool(message, field));
      } break;
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        if (!JsonOfMessage(
                field->is_repeated()
                    ? reflection->GetRepeatedMessage(message, field, i)
                    : reflection->GetMessage(message, field),
                writer)) {
          return false;
        }
      } break;
      case FieldDescriptor::CPPTYPE_INT32: {
        writer->Int(field->is_repeated()
                        ? reflection->GetRepeatedInt32(message, field, i)
                        : reflection->GetInt32(message, field));
      } break;
      default:
        return false;
    }
  }
  if (field->is_repeated()) {
    writer->EndArray();
  }
  return true;
}

/// \tparam W A RapidJSON Writer.
template <typename W>
bool JsonOfMessage(const google::protobuf::Message &message, W *writer) {
  using namespace google::protobuf;
  writer->StartObject();
  auto *descriptor = message.GetDescriptor();
  auto *reflection = message.GetReflection();
  std::vector<const FieldDescriptor *> fields;
  reflection->ListFields(message, &fields);
  for (int i = 0; i < descriptor->field_count(); ++i) {
    auto *field = descriptor->field(i);
    if (field->is_repeated() && reflection->FieldSize(message, field) == 0) {
      fields.push_back(field);
    }
  }
  for (auto *field : fields) {
    if (!field->is_repeated() && !reflection->HasField(message, field)) {
      continue;
    }
    if (!JsonOfValue(field, message, reflection, writer)) {
      return false;
    }
  }
  writer->EndObject();
  return true;
}

bool WriteMessageAsJsonToString(const google::protobuf::Message &message,
                                std::string *out) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  if (!JsonOfMessage(message, &writer)) {
    return false;
  }
  *out = buffer.GetString();
  return true;
}

bool WriteMessageAsJsonToString(const google::protobuf::Message &message,
                                const std::string &format_key,
                                std::string *out) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartObject();
  writer.Key("format");
  writer.String(format_key.c_str());
  writer.Key("content");
  if (!JsonOfMessage(message, &writer)) {
    return false;
  }
  writer.EndObject();
  *out = buffer.GetString();
  return true;
}

bool MessageOfJson(const rapidjson::Value &value,
                   google::protobuf::Message *message) {
  using namespace rapidjson;
  using namespace google::protobuf;
  auto *descriptor = message->GetDescriptor();
  auto *reflection = message->GetReflection();
  if (!value.IsObject()) {
    return false;
  }
  for (auto field = value.MemberBegin(); field != value.MemberEnd(); ++field) {
    const Value &field_name = field->name;
    if (!field_name.IsString()) {
      return false;
    }
    const auto *proto_field =
        descriptor->FindFieldByName(field_name.GetString());
    if (!proto_field) {
      // Ignore unknown fields.
      continue;
    }
    if (proto_field->is_repeated()) {
      if (!field->value.IsArray()) {
        return false;
      }
      for (auto data = field->value.Begin(); data != field->value.End();
           ++data) {
        switch (proto_field->cpp_type()) {
          case FieldDescriptor::CPPTYPE_INT32:
            if (!data->IsInt()) {
              return false;
            }
            reflection->AddInt32(message, proto_field, data->GetInt());
            break;
          case FieldDescriptor::CPPTYPE_STRING:
            if (!data->IsString()) {
              return false;
            }
            if (proto_field->type() == FieldDescriptor::TYPE_BYTES) {
              google::protobuf::string buffer;
              if (!DecodeBase64(data->GetString(), &buffer)) {
                return false;
              }
              reflection->AddString(message, proto_field, buffer);
            } else {
              reflection->AddString(message, proto_field, data->GetString());
            }
            break;
          case FieldDescriptor::CPPTYPE_BOOL:
            if (!data->IsBool()) {
              return false;
            }
            reflection->AddBool(message, proto_field, data->GetBool());
            break;
          case FieldDescriptor::CPPTYPE_MESSAGE:
            if (!MessageOfJson(*data,
                               reflection->AddMessage(message, proto_field))) {
              return false;
            }
            break;
          default:
            return false;
        }
      }
    } else {
      switch (proto_field->cpp_type()) {
        case FieldDescriptor::CPPTYPE_STRING:
          if (!field->value.IsString()) {
            return false;
          }
          if (proto_field->type() == FieldDescriptor::TYPE_BYTES) {
            google::protobuf::string buffer;
            if (!DecodeBase64(field->value.GetString(), &buffer)) {
              return false;
            }
            reflection->SetString(message, proto_field, buffer);
          } else {
            reflection->SetString(message, proto_field,
                                  field->value.GetString());
          }
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          if (!field->value.IsBool()) {
            return false;
          }
          reflection->SetBool(message, proto_field, field->value.GetBool());
          break;
        case FieldDescriptor::CPPTYPE_INT32:
          if (!field->value.IsInt()) {
            return false;
          }
          reflection->SetInt32(message, proto_field, field->value.GetInt());
          break;
        case FieldDescriptor::CPPTYPE_MESSAGE:
          if (!MessageOfJson(field->value, reflection->MutableMessage(
                                               message, proto_field))) {
            return false;
          }
          break;
        default:
          return false;
      }
    }
  }
  return true;
}

bool MergeJsonWithMessage(const std::string &in, std::string *format_key,
                          google::protobuf::Message *message) {
  rapidjson::Document document;
  document.Parse(in.c_str());
  if (document.HasParseError()) {
    return false;
  }
  if (!document.IsObject() || !document.HasMember("format") ||
      !document.HasMember("content") || !document["format"].IsString() ||
      !document["content"].IsObject()) {
    return false;
  }
  std::string in_format = document["format"].GetString();
  if (format_key) {
    *format_key = in_format;
  }
  if (in_format == "kythe") {
    return MessageOfJson(document["content"], message);
  }
  return false;
}

void PackAny(const google::protobuf::Message &message, const char *type_uri,
             google::protobuf::Any *out) {
  out->set_type_url(type_uri);
  google::protobuf::io::StringOutputStream stream(out->mutable_value());
  google::protobuf::io::CodedOutputStream coded_output_stream(&stream);
  message.SerializeToCodedStream(&coded_output_stream);
}

bool UnpackAny(const google::protobuf::Any &any,
               google::protobuf::Message *result) {
  google::protobuf::io::ArrayInputStream stream(any.value().data(),
                                                any.value().size());
  google::protobuf::io::CodedInputStream coded_input_stream(&stream);
  return result->ParseFromCodedStream(&coded_input_stream);
}
}  // namespace kythe
