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

#include "kythe/cxx/common/net_client.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <rapidjson/document.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message.h"
#include "kythe/cxx/common/json_proto.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace kythe {

JsonClient::JsonClient() : curl_(::curl_easy_init()) {
  CHECK(curl_ != nullptr);
}

JsonClient::~JsonClient() {
  if (curl_) {
    ::curl_easy_cleanup(curl_);
    curl_ = nullptr;
  }
}

void JsonClient::InitNetwork() {
  CHECK(::curl_global_init(CURL_GLOBAL_ALL) == 0);
}

size_t JsonClient::CurlWriteCallback(void* data, size_t size, size_t nmemb,
                                     void* user) {
  JsonClient* client = static_cast<JsonClient*>(user);
  size_t receive_head = client->received_.size();
  client->received_.resize(receive_head + size * nmemb);
  ::memcpy(&client->received_[receive_head], data, size * nmemb);
  return size * nmemb;
}

size_t JsonClient::CurlReadCallback(void* data, size_t size, size_t nmemb,
                                    void* user) {
  JsonClient* client = static_cast<JsonClient*>(user);
  if (client->send_head_ >= client->to_send_.size()) {
    return 0;
  }
  size_t bytes_to_send =
      std::min(size * nmemb, client->to_send_.size() - client->send_head_);
  ::memcpy(data, client->to_send_.data() + client->send_head_, bytes_to_send);
  client->send_head_ += bytes_to_send;
  return bytes_to_send;
}

bool JsonClient::Request(const std::string& uri, bool post,
                         const rapidjson::Document& request,
                         rapidjson::Document* response) {
  rapidjson::StringBuffer string_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
  request.Accept(writer);
  return Request(uri, post, string_buffer.GetString(), response);
}

bool JsonClient::Request(const std::string& uri, bool post,
                         const std::string& request,
                         rapidjson::Document* response) {
  std::string to_decode;
  if (!Request(uri, post, request, &to_decode)) {
    return false;
  }
  response->Parse(to_decode.c_str());
  if (response->HasParseError()) {
    LOG(ERROR) << "(uri: " << uri << "): bad JSON at offset "
               << response->GetErrorOffset() << ": "
               << rapidjson::GetParseError_En(response->GetParseError());
    return false;
  }
  return true;
}

bool JsonClient::Request(const std::string& uri, bool post,
                         const std::string& request, std::string* response) {
  to_send_ = request;
  send_head_ = 0;
  received_.clear();

  ::curl_easy_setopt(curl_, CURLOPT_URL, uri.c_str());
  ::curl_easy_setopt(curl_, CURLOPT_POST, post ? 1L : 0L);
  ::curl_easy_setopt(curl_, CURLOPT_READFUNCTION, CurlReadCallback);
  ::curl_easy_setopt(curl_, CURLOPT_READDATA, this);
  ::curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  ::curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);
  ::curl_slist* headers = nullptr;
  if (post) {
    headers = ::curl_slist_append(headers, "Content-Type: application/json");
    ::curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    ::curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, request.size());
  }

  ::CURLcode res = ::curl_easy_perform(curl_);
  ::curl_slist_free_all(headers);

  if (res) {
    LOG(ERROR) << "(uri: " << uri << "): " << ::curl_easy_strerror(res);
    return false;
  }

  long response_code;
  res = ::curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
  if (res) {
    LOG(ERROR) << "(uri: " << uri << "): " << ::curl_easy_strerror(res);
    return false;
  }
  if (response_code != 200) {
    LOG(ERROR) << "(uri: " << uri << "): response " << response_code;
    return false;
  }
  if (response) {
    *response = received_;
  }
  return true;
}

bool XrefsJsonClient::Roundtrip(const std::string& endpoint,
                                const google::protobuf::Message& request,
                                google::protobuf::Message* response,
                                std::string* error_text) {
  std::string request_json;
  if (!WriteMessageAsJsonToString(request, &request_json)) {
    if (error_text) {
      *error_text = "Couldn't serialize message.";
    }
    return false;
  }
  std::string response_buffer;
  if (!client_->Request(endpoint, true, request_json, &response_buffer)) {
    if (error_text) {
      *error_text = "Network client error.";
    }
    return false;
  }
  if (response) {
    google::protobuf::io::ArrayInputStream stream(response_buffer.data(),
                                                  response_buffer.size());
    google::protobuf::io::CodedInputStream coded_stream(&stream);
    if (!response->ParseFromCodedStream(&coded_stream)) {
      if (error_text) {
        *error_text = "Error decoding response protobuf.";
      }
      return false;
    }
  }
  return true;
}
}  // namespace kythe
