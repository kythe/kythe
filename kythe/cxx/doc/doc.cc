/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// doc is a utility that performs simple formatting tasks on documentation
// extracted from the Kythe graph.

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/net_client.h"
#include "kythe/cxx/doc/html_renderer.h"

DEFINE_string(xrefs, "http://localhost:8080", "Base URI for xrefs service");
DEFINE_string(corpus, "test", "Default corpus to use");
DEFINE_string(file, "", "Process all documented nodes in this file");

namespace kythe {
namespace {
const char kDocHeader[] = R"(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Kythe doc output</title>
  </head>
  <body>
)";
const char kDocFooter[] = R"(
  </body>
</html>
)";
const char kDefinesBinding[] = "/kythe/edge/defines/binding";

int DocumentNodesFrom(XrefsJsonClient* client, const proto::VName& file_name) {
  proto::DecorationsRequest request;
  proto::DecorationsReply reply;
  request.mutable_location()->set_ticket(URI(file_name).ToString());
  request.set_references(true);
  std::string error;
  CHECK(client->Decorations(request, &reply, &error)) << error;
  proto::DocumentationRequest doc_request;
  proto::DocumentationReply doc_reply;
  for (const auto& reference : reply.reference()) {
    if (reference.kind() == kDefinesBinding) {
      doc_request.add_ticket(reference.target_ticket());
    }
  }
  CHECK(client->Documentation(doc_request, &doc_reply, &error)) << error;
  HtmlRendererOptions options;
  options.make_link_uri = [](const proto::Anchor& anchor) {
    return anchor.parent();
  };
  ::fputs(kDocHeader, stdout);
  for (const auto& document : doc_reply.document()) {
    auto html = RenderHtml(options, document);
    ::fputs(html.c_str(), stdout);
  }
  ::fputs(kDocFooter, stdout);
  return 0;
}
}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetUsageMessage(R"(perform simple documentation formatting

doc -corpus foo -file bar.cc
  Formats documentation for all nodes attached via defines/binding anchors to
  a file with path bar.cc in corpus foo.
)");
  google::ParseCommandLineFlags(&argc, &argv, true);
  kythe::JsonClient::InitNetwork();
  kythe::XrefsJsonClient client(
      std::unique_ptr<kythe::JsonClient>(new kythe::JsonClient()), FLAGS_xrefs);
  if (!FLAGS_file.empty()) {
    auto ticket = kythe::URI::FromString(FLAGS_file);
    if (!ticket.first) {
      ticket = kythe::URI::FromString(
          "kythe://" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths, FLAGS_corpus) +
          "?path=" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths, FLAGS_file));
    }
    if (!ticket.first) {
      ::fprintf(stderr, "Couldn't parse URI %s\n", FLAGS_file.c_str());
      return 1;
    }
    return kythe::DocumentNodesFrom(&client, ticket.second.v_name());
  }
  return 0;
}
