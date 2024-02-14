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

#ifndef KYTHE_CXX_COMMON_JSON_PROTO_H_
#define KYTHE_CXX_COMMON_JSON_PROTO_H_

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/any.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"

namespace kythe {

/// \brief Deserializes a protobuf from JSON text.
/// \param input The input text to parse.
/// \param message The message to parse.
/// \return The status message result of parsing.
absl::Status ParseFromJsonString(absl::string_view input,
                                 google::protobuf::Message* message);

/// \brief Deserializes a protobuf from JSON text.
/// \param input The input text to parse.
/// \param options The JsonParseOptions to use.
/// \param message The message to parse.
/// \return The status message result of parsing.
absl::Status ParseFromJsonString(
    absl::string_view input,
    const google::protobuf::util::JsonParseOptions& options,
    google::protobuf::Message* message);

/// \brief Deserializes a protobuf from a JSON text stream.
/// \param stream The input stream from which to read.
/// \param message The message to parse.
/// \return The status message result of parsing.
absl::Status ParseFromJsonStream(
    google::protobuf::io::ZeroCopyInputStream* input,
    google::protobuf::Message* message);

/// \brief Deserializes a protobuf from a JSON text stream.
/// \param input The input stream from which to read.
/// \param options The JsonParseOptions to use.
/// \param message The message to parse.
/// \return The status message result of parsing.
absl::Status ParseFromJsonStream(
    google::protobuf::io::ZeroCopyInputStream* input,
    const google::protobuf::util::JsonParseOptions& options,
    google::protobuf::Message* message);

/// \brief Serializes a protobuf to JSON form, including the format wrapper.
/// \param message The protobuf to serialize.
/// \param format_key Specifies the format to declare in the wrapper.
/// \param out Set to the serialized message on success.
/// \return True on success; false on failure.
bool WriteMessageAsJsonToString(const google::protobuf::Message& message,
                                const std::string& format_key,
                                std::string* out);

/// \brief Serializes a protobuf to JSON form with no wrapper.
/// \param message The protobuf to serialize.
/// \param out Set to the serialized message on success.
/// \return True on success; false on failure.
bool WriteMessageAsJsonToString(const google::protobuf::Message& message,
                                std::string* out);

/// \brief Serializes a protobuf to JSON form with no wrapper.
/// \param message The protobuf to serialize.
/// \return JSON string on success; Status on failure.
absl::StatusOr<std::string> WriteMessageAsJsonToString(
    const google::protobuf::Message& message);

/// \brief Wrap a protobuf up into an Any.
/// \param message The message to wrap.
/// \param type_uri The URI of the message type.
/// \param out The resulting Any.
void PackAny(const google::protobuf::Message& message,
             absl::string_view type_uri, google::protobuf::Any* out);

/// \brief Unpack a protobuf from an Any.
/// \param any The Any to unpack.
/// \param result The message to unpack it over.
/// \return false if unpacking failed
bool UnpackAny(const google::protobuf::Any& any,
               google::protobuf::Message* result);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_JSON_PROTO_H_
