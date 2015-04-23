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

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/cxx/common/net_client.h"
#include "kythe/cxx/tools/fyi/fyi.h"
#include "llvm/Support/CommandLine.h"

// We use the default support in Clang for compilation databases.

namespace cl = llvm::cl;
static cl::extrahelp common_help(
    clang::tooling::CommonOptionsParser::HelpMessage);
static cl::OptionCategory fyi_options("Tool options");
static cl::opt<std::string> xrefs("xrefs",
                                  cl::desc("Base URI for xrefs service"),
                                  cl::init("http://localhost:8080"));

int main(int argc, const char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetVersionString("0.1");
  google::SetUsageMessage("fyi: repair a C++ file with missing includes");
  clang::tooling::CommonOptionsParser options(argc, argv, fyi_options);
  kythe::JsonClient::InitNetwork();
  auto xrefs_db = llvm::make_unique<kythe::XrefsJsonClient>(
      llvm::make_unique<kythe::JsonClient>(), xrefs);
  clang::tooling::ClangTool tool(options.getCompilations(),
                                 options.getSourcePathList());
  kythe::fyi::ActionFactory factory(std::move(xrefs_db), 5);
  return tool.run(&factory);
}
