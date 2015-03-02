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

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/net_client.h"

DEFINE_string(xrefs, "http://localhost:8080", "Base URI for xrefs service");

namespace {
void TestNodeRequest() {
  kythe::XrefsJsonClient client(
      std::unique_ptr<kythe::JsonClient>(new kythe::JsonClient()), FLAGS_xrefs);
  kythe::proto::NodesRequest request;
  kythe::proto::NodesReply response;
  // TODO(zarko): Use kythe::URI once it's merged in.
  request.add_ticket("kythe:?lang=c%2B%2B#SOMEFILE");
  std::string error;
  CHECK(client.Nodes(request, &response, &error)) << error;
  CHECK_EQ(1, response.node_size()) << response.DebugString();
  CHECK_EQ(1, response.node(0).fact_size()) << response.DebugString();
  CHECK_EQ(request.ticket(0), response.node(0).ticket())
      << response.DebugString();
  CHECK_EQ("/kythe/node/kind", response.node(0).fact(0).name());
  CHECK_EQ("file", response.node(0).fact(0).value());
}
}

int main(int argc, char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);
  kythe::JsonClient::InitNetwork();
  TestNodeRequest();
  return 0;
}
