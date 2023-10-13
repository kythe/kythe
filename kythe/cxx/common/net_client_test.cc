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

#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/graph.pb.h"

ABSL_FLAG(std::string, xrefs, "http://localhost:8080",
          "Base URI for xrefs service");

namespace {
void TestNodeRequest() {
  kythe::XrefsJsonClient client(std::make_unique<kythe::JsonClient>(),
                                absl::GetFlag(FLAGS_xrefs));
  kythe::proto::NodesRequest request;
  kythe::proto::NodesReply response;
  // TODO(zarko): Use kythe::URI once it's merged in.
  request.add_ticket("kythe:?lang=c%2B%2B#SOMEFILE");
  std::string error;
  CHECK(client.Nodes(request, &response, &error)) << error;
  CHECK_EQ(1, response.nodes().size()) << response;

  CHECK_EQ(request.ticket(0), response.nodes().begin()->first) << response;
  kythe::proto::common::NodeInfo node = response.nodes().begin()->second;
  CHECK_EQ(1, node.facts().size()) << response;

  CHECK_EQ("/kythe/node/kind", node.facts().begin()->first);
  CHECK_EQ("file", node.facts().begin()->second);
}
}  // namespace

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  absl::ParseCommandLine(argc, argv);
  kythe::JsonClient::InitNetwork();
  TestNodeRequest();
  return 0;
}
