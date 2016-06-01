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

#include <fcntl.h>
#include <sys/stat.h>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/net_client.h"
#include "kythe/cxx/doc/html_renderer.h"
#include "kythe/cxx/doc/javadoxygen_markup_handler.h"
#include "kythe/cxx/doc/markup_handler.h"

DEFINE_string(xrefs, "http://localhost:8080", "Base URI for xrefs service");
DEFINE_string(corpus, "test", "Default corpus to use");
DEFINE_string(path, "",
              "Look up this path in the xrefs service and process all "
              "documented nodes inside");
DEFINE_string(save_response, "",
              "Save the initial documentation response to this file as an "
              "ASCII protobuf.");

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

int DocumentNodesFrom(const proto::DocumentationReply& doc_reply) {
  HtmlRendererOptions options;
  options.make_link_uri = [](const proto::Anchor& anchor) {
    return anchor.parent();
  };
  ::fputs(kDocHeader, stdout);
  for (const auto& document : doc_reply.document()) {
    if (document.has_text()) {
      Printable printable(document.text());
      auto markdoc = HandleMarkup({ParseJavadoxygen}, printable);
      auto html = RenderHtml(options, markdoc);
      ::fputs(html.c_str(), stdout);
    }
  }
  ::fputs(kDocFooter, stdout);
  return 0;
}

int DocumentNodesFromStdin() {
  proto::DocumentationReply doc_reply;
  google::protobuf::io::FileInputStream file_input_stream(STDIN_FILENO);
  CHECK(google::protobuf::TextFormat::Parse(&file_input_stream, &doc_reply));
  return DocumentNodesFrom(doc_reply);
}

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
  fprintf(stderr, "Looking for %d tickets\n", doc_request.ticket_size());
  CHECK(client->Documentation(doc_request, &doc_reply, &error)) << error;
  if (!FLAGS_save_response.empty()) {
    int saved =
        open(FLAGS_save_response.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if (saved < 0) {
      fprintf(stderr, "Couldn't open %s\n", FLAGS_save_response.c_str());
      return 1;
    }
    {
      google::protobuf::io::FileOutputStream outfile(saved);
      if (!google::protobuf::TextFormat::Print(doc_reply, &outfile)) {
        fprintf(stderr, "Coudln't print to %s\n", FLAGS_save_response.c_str());
        close(saved);
        return 1;
      }
    }
    if (close(saved) < 0) {
      fprintf(stderr, "Couldn't close %s\n", FLAGS_save_response.c_str());
      return 1;
    }
  }
  return DocumentNodesFrom(doc_reply);
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
doc
  Formats documentation from a text-format proto::DocumentationReply provided
  on standard input.
)");
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_path.empty()) {
    return kythe::DocumentNodesFromStdin();
  } else {
    kythe::JsonClient::InitNetwork();
    kythe::XrefsJsonClient client(
        std::unique_ptr<kythe::JsonClient>(new kythe::JsonClient()),
        FLAGS_xrefs);
    auto ticket = kythe::URI::FromString(FLAGS_path);
    if (!ticket.first) {
      ticket = kythe::URI::FromString(
          "kythe://" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths, FLAGS_corpus) +
          "?path=" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths, FLAGS_path));
    }
    if (!ticket.first) {
      ::fprintf(stderr, "Couldn't parse URI %s\n", FLAGS_path.c_str());
      return 1;
    }
    return kythe::DocumentNodesFrom(&client, ticket.second.v_name());
  }
  return 0;
}
