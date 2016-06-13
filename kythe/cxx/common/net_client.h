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

#ifndef KYTHE_CXX_COMMON_NET_CLIENT_H_
#define KYTHE_CXX_COMMON_NET_CLIENT_H_

#include <memory>
#include <string>

#include <curl/curl.h>

#include "kythe/proto/storage.pb.h"
#include "kythe/proto/xref.pb.h"
#include "rapidjson/document.h"

namespace kythe {

/// \brief Issues JSON-formatted RPCs.
///
/// JsonClient is not thread-safe.
class JsonClient {
 public:
  JsonClient();
  ~JsonClient();

  /// \brief Call once to initialize the underlying network library.
  static void InitNetwork();

  /// \brief Issue a request.
  /// \param uri The URI to request.
  /// \param post Issue this request as a post?
  /// \param request The document to issue as the request.
  /// \param response The document to fill with the response.
  /// \return true on success and false on failure
  bool Request(const std::string &uri, bool post,
               const rapidjson::Document &request,
               rapidjson::Document *response);

  /// \brief Issue a request.
  /// \param uri The URI to request.
  /// \param post Issue this request as a post?
  /// \param request The string to issue as the request.
  /// \param response The document to fill with the response.
  /// \return true on success and false on failure
  bool Request(const std::string &uri, bool post, const std::string &request,
               rapidjson::Document *response);

  /// \brief Issue a request.
  /// \param uri The URI to request.
  /// \param post Issue this request as a post?
  /// \param request The string to issue as the request.
  /// \param response The raw string to fill with the response.
  /// \return true on success and false on failure
  bool Request(const std::string &uri, bool post, const std::string &request,
               std::string *response);

 private:
  static size_t CurlWriteCallback(void *data, size_t size, size_t nmemb,
                                  void *user);
  static size_t CurlReadCallback(void *data, size_t size, size_t nmemb,
                                 void *user);

  /// The network context.
  CURL *curl_;
  /// A buffer used for communications.
  std::string to_send_;
  /// Where we are in the buffer.
  size_t send_head_;
  /// A buffer used for communications.
  std::string received_;
};

/// \brief A client for a Kythe xrefs service.
class XrefsClient {
 public:
  virtual ~XrefsClient() {}

  /// \brief Issues a Nodes call.
  /// \param request The request to send.
  /// \param reply On success, will be merged with the reply.
  /// \param error_text On failure, will be set to an error description.
  /// \return true on success, false on failure.
  virtual bool Nodes(const proto::NodesRequest &request,
                     proto::NodesReply *reply, std::string *error_text) {
    if (error_text) {
      *error_text = "Unimplemented.";
    }
    return false;
  }

  /// \brief Issues an Edges call.
  /// \param request The request to send.
  /// \param reply On success, will be merged with the reply.
  /// \param error_text On failure, will be set to an error description.
  /// \return true on success, false on failure.
  virtual bool Edges(const proto::EdgesRequest &request,
                     proto::EdgesReply *reply, std::string *error_text) {
    if (error_text) {
      *error_text = "Unimplemented.";
    }
    return false;
  }

  /// \brief Issues a Decorations call.
  /// \param request The request to send.
  /// \param reply On success, will be merged with the reply.
  /// \param error_text On failure, will be set to an error description.
  /// \return true on success, false on failure.
  virtual bool Decorations(const proto::DecorationsRequest &request,
                           proto::DecorationsReply *reply,
                           std::string *error_text) {
    if (error_text) {
      *error_text = "Unimplemented.";
    }
    return false;
  }

  /// \brief Issues a Documentation call.
  /// \param request The request to send.
  /// \param reply On success, will be merged with the reply.
  /// \param error_text On failure, will be set to an error description.
  /// \return true on success, false on failure.
  virtual bool Documentation(const proto::DocumentationRequest &request,
                             proto::DocumentationReply *reply,
                             std::string *error_text) {
    if (error_text) {
      *error_text = "Unimplemented.";
    }
    return false;
  }
};

/// \brief A client for a Kythe xrefs service that talks JSON.
class XrefsJsonClient : public XrefsClient {
 public:
  /// \param client The JsonClient to use.
  /// \param base_uri The base URI of the service ("http://localhost:8080")
  XrefsJsonClient(std::unique_ptr<JsonClient> client,
                  const std::string &base_uri)
      : client_(std::move(client)),
        nodes_uri_(base_uri + "/nodes?proto=1"),
        edges_uri_(base_uri + "/edges?proto=1"),
        decorations_uri_(base_uri + "/decorations?proto=1"),
        documentation_uri_(base_uri + "/documentation?proto=1") {}
  bool Nodes(const proto::NodesRequest &request, proto::NodesReply *reply,
             std::string *error_text) override {
    return Roundtrip(nodes_uri_, request, reply, error_text);
  }
  bool Edges(const proto::EdgesRequest &request, proto::EdgesReply *reply,
             std::string *error_text) override {
    return Roundtrip(edges_uri_, request, reply, error_text);
  }
  bool Decorations(const proto::DecorationsRequest &request,
                   proto::DecorationsReply *reply,
                   std::string *error_text) override {
    return Roundtrip(decorations_uri_, request, reply, error_text);
  }
  bool Documentation(const proto::DocumentationRequest &request,
                     proto::DocumentationReply *reply,
                     std::string *error_text) override {
    return Roundtrip(documentation_uri_, request, reply, error_text);
  }

 private:
  bool Roundtrip(const std::string &endpoint,
                 const google::protobuf::Message &request,
                 google::protobuf::Message *response, std::string *error_text);

  std::unique_ptr<JsonClient> client_;
  std::string nodes_uri_;
  std::string edges_uri_;
  std::string decorations_uri_;
  std::string documentation_uri_;
};
}

#endif  // KYTHE_CXX_COMMON_NET_CLIENT_H_
