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

#ifndef KYTHE_CXX_COMMON_JSON_PROTO_H_
#define KYTHE_CXX_COMMON_JSON_PROTO_H_

#include <string>

#include "google/protobuf/message.h"
#include "kythe/proto/any.pb.h"
#include "rapidjson/document.h"

namespace kythe {

/// \brief Deserializes a protobuf from its JSON form, including the format
/// wrapper.
/// \param in The string to deserialize.
/// \param format_key Set to the wrapper's format field.
/// \param message Merged with the JSON data.
/// \return true on success; false on failure.
bool MergeJsonWithMessage(const std::string &in, std::string *format_key,
                          google::protobuf::Message *message);

/// \brief Deserializes a protobuf from its JSON form without expecting a
/// wrapper.
/// \param document The document to deserialize.
/// \param message Merged with the JSON data.
/// \return true on success; false on failure.
bool MergeJsonWithMessage(const rapidjson::Document &document,
                          google::protobuf::Message *message);

/// \brief Serializes a protobuf to JSON form, including the format wrapper.
/// \param message The protobuf to serialize.
/// \param format_key Specifies the format to declare in the wrapper.
/// \param out Set to the serialized message on success.
/// \return True on success; false on failure.
bool WriteMessageAsJsonToString(const google::protobuf::Message &message,
                                const std::string &format_key,
                                std::string *out);

/// \brief Serializes a protobuf to JSON form with no wrapper.
/// \param message The protobuf to serialize.
/// \param out Set to the serialized message on success.
/// \return True on success; false on failure.
bool WriteMessageAsJsonToString(const google::protobuf::Message &message,
                                std::string *out);

/// \brief Wrap a protobuf up into an Any.
/// \param message The message to wrap.
/// \param type_uri The URI of the message type.
/// \param out The resulting Any.
void PackAny(const google::protobuf::Message &message, const char *type_uri,
             kythe::proto::Any *out);

/// \brief Unpack a protobuf from an Any.
/// \param any The Any to unpack.
/// \param result The message to unpack it over.
/// \return false if unpacking failed
bool UnpackAny(const kythe::proto::Any &any, google::protobuf::Message *result);

/// \brief Decodes a base64-encoded string.
/// \param data The string to decode.
/// \param decoded Set to the decoded value.
/// \return false on failure.
bool DecodeBase64(const google::protobuf::string &data,
                  google::protobuf::string *decoded);

/// \brief Encodes a string as base64.
/// \param data The string to encode.
/// \param encoded Set to the encoded value.
google::protobuf::string EncodeBase64(const google::protobuf::string &data);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_JSON_PROTO_H_
