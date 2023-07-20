/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/net_client.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/cxx/common/schema/facts.h"
#include "kythe/cxx/doc/html_markup_handler.h"
#include "kythe/cxx/doc/html_renderer.h"
#include "kythe/cxx/doc/javadoxygen_markup_handler.h"
#include "kythe/cxx/doc/markup_handler.h"

ABSL_FLAG(std::string, xrefs, "http://localhost:8080",
          "Base URI for xrefs service");
ABSL_FLAG(std::string, corpus, "test", "Default corpus to use");
ABSL_FLAG(std::string, path, "",
          "Look up this path in the xrefs service and process all "
          "documented nodes inside");
ABSL_FLAG(std::string, save_response, "",
          "Save the initial documentation response to this file as an "
          "ASCII protobuf.");
ABSL_FLAG(std::string, css, "",
          "Include this stylesheet path in the resulting HTML.");
ABSL_FLAG(bool, common_signatures, false,
          "Render the MarkedSource proto from standard in.");

namespace kythe {
namespace {
constexpr char kDocHeaderPrefix[] = R"(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
)";
constexpr char kDocHeaderSuffix[] = R"(    <title>Kythe doc output</title>
  </head>
  <body>
)";
constexpr char kDocFooter[] = R"(
  </body>
</html>
)";

int DocumentNodesFrom(const proto::DocumentationReply& doc_reply) {
  ::fputs(kDocHeaderPrefix, stdout);
  if (!absl::GetFlag(FLAGS_css).empty()) {
    absl::FPrintF(stdout,
                  "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">",
                  absl::GetFlag(FLAGS_css));
  }
  ::fputs(kDocHeaderSuffix, stdout);
  DocumentHtmlRendererOptions options(doc_reply);
  options.make_link_uri = [](const proto::Anchor& anchor) {
    return anchor.parent();
  };
  options.kind_name = [&options](const std::string& ticket) {
    if (const auto* node = options.node_info(ticket)) {
      for (const auto& fact : node->facts()) {
        if (fact.first == kythe::common::schema::kFactNodeKind) {
          return std::string(fact.second);
        }
      }
    }
    return std::string();
  };
  for (const auto& document : doc_reply.document()) {
    if (document.has_text()) {
      auto html =
          RenderDocument(options, {ParseJavadoxygen, ParseHtml}, document);
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

int RenderMarkedSourceFromStdin() {
  proto::common::MarkedSource marked_source;
  google::protobuf::io::FileInputStream file_input_stream(STDIN_FILENO);
  CHECK(
      google::protobuf::TextFormat::Parse(&file_input_stream, &marked_source));
  absl::PrintF("      RenderSimpleIdentifier: \"%s\"\n",
               RenderSimpleIdentifier(marked_source));
  auto params = RenderSimpleParams(marked_source);
  for (const auto& param : params) {
    absl::PrintF("          RenderSimpleParams: \"%s\"\n", param);
  }
  absl::PrintF("RenderSimpleQualifiedName-ID: \"%s\"\n",
               RenderSimpleQualifiedName(marked_source, false));
  absl::PrintF("RenderSimpleQualifiedName+ID: \"%s\"\n",
               RenderSimpleQualifiedName(marked_source, true));
  return 0;
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
    if (reference.kind() == kythe::common::schema::kDefinesBinding) {
      doc_request.add_ticket(reference.target_ticket());
    }
  }
  absl::FPrintF(stderr, "Looking for %d tickets\n", doc_request.ticket_size());
  CHECK(client->Documentation(doc_request, &doc_reply, &error)) << error;
  if (!absl::GetFlag(FLAGS_save_response).empty()) {
    int saved = open(absl::GetFlag(FLAGS_save_response).c_str(),
                     O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if (saved < 0) {
      absl::FPrintF(stderr, "Couldn't open %s\n",
                    absl::GetFlag(FLAGS_save_response));
      return 1;
    }
    {
      google::protobuf::io::FileOutputStream outfile(saved);
      if (!google::protobuf::TextFormat::Print(doc_reply, &outfile)) {
        absl::FPrintF(stderr, "Coudln't print to %s\n",
                      absl::GetFlag(FLAGS_save_response).c_str());
        close(saved);
        return 1;
      }
    }
    if (close(saved) < 0) {
      absl::FPrintF(stderr, "Couldn't close %s\n",
                    absl::GetFlag(FLAGS_save_response));
      return 1;
    }
  }
  return DocumentNodesFrom(doc_reply);
}
}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(R"(perform simple documentation formatting

doc -corpus foo -file bar.cc
  Formats documentation for all nodes attached via defines/binding anchors to
  a file with path bar.cc in corpus foo.
doc
  Formats documentation from a text-format proto::DocumentationReply provided
  on standard input.
doc -common_signatures
  Renders the text-format proto::common::MarkedSource message provided on standard
  input into several common forms.
)");
  absl::ParseCommandLine(argc, argv);
  if (absl::GetFlag(FLAGS_common_signatures)) {
    return kythe::RenderMarkedSourceFromStdin();
  } else if (absl::GetFlag(FLAGS_path).empty()) {
    return kythe::DocumentNodesFromStdin();
  } else {
    kythe::JsonClient::InitNetwork();
    kythe::XrefsJsonClient client(std::make_unique<kythe::JsonClient>(),
                                  absl::GetFlag(FLAGS_xrefs));
    auto ticket = kythe::URI::FromString(absl::GetFlag(FLAGS_path));
    if (!ticket.first) {
      ticket = kythe::URI::FromString(
          "kythe://" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths,
                           absl::GetFlag(FLAGS_corpus)) +
          "?path=" +
          kythe::UriEscape(kythe::UriEscapeMode::kEscapePaths,
                           absl::GetFlag(FLAGS_path)));
    }
    if (!ticket.first) {
      absl::FPrintF(stderr, "Couldn't parse URI %s\n",
                    absl::GetFlag(FLAGS_path));
      return 1;
    }
    return kythe::DocumentNodesFrom(&client, ticket.second.v_name());
  }
  return 0;
}
