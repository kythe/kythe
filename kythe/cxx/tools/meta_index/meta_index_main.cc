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

#include "llvm/Support/Program.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/cxx/common/net_client.h"

#include "commonjs_package_registry.h"
#include "meta_index.h"
#include "vcs.h"

DEFINE_string(commonjs_registry, "https://skimdb.npmjs.com/registry",
              "The CommonJS registry to use.");
DEFINE_string(commonjs_registry_cache, "",
              "Local file to store a cache of CommonJS registry queries.");
DEFINE_string(vcs_cache, "", "Local file to store a cache of VCS metadata.");
DEFINE_string(vnames, "", "Path to save vnames.json for all repositories.");
DEFINE_string(shout, "", "Path to save indexing script.");
DEFINE_string(repo_subgraph, "", "Path to save repo metadata subgraph.");
DEFINE_bool(allow_network, false, "Allow network access.");
DEFINE_bool(log_to_logfile, false, "Don't force --logtostderr.");
DEFINE_string(default_local_vcs_id, "", "Use a default VCS ID for local paths");

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  if (!FLAGS_log_to_logfile) {
    FLAGS_logtostderr = true;
  }
  google::InitGoogleLogging(argv[0]);

  google::SetVersionString("0.1");
  google::SetUsageMessage("meta_index: index your various repositories");
  google::ParseCommandLineFlags(&argc, &argv, true);

  kythe::JsonClient::InitNetwork();
  std::unique_ptr<kythe::CommonJsPackageRegistryClient> registry;
  if (FLAGS_commonjs_registry_cache.empty()) {
    registry.reset(
        new kythe::CommonJsPackageRegistryClient(FLAGS_commonjs_registry));
  } else {
    registry.reset(new kythe::CommonJsPackageRegistryCachingClient(
        FLAGS_commonjs_registry, FLAGS_commonjs_registry_cache));
  }
  registry->set_allow_network(FLAGS_allow_network);
  std::unique_ptr<kythe::VcsClient> vcs(new kythe::VcsClient(FLAGS_vcs_cache));
  vcs->set_allow_network(FLAGS_allow_network);

  kythe::Repositories repos(clang::vfs::getRealFileSystem(),
                            std::move(registry), std::move(vcs));
  if (!FLAGS_default_local_vcs_id.empty()) {
    repos.set_default_vcs_id(FLAGS_default_local_vcs_id);
  }
  for (int arg = 1; arg < argc; ++arg) {
    repos.AddRepositoryRoot(argv[arg]);
  }
  if (!FLAGS_vnames.empty()) {
    repos.ExportVNamesJson(FLAGS_vnames.c_str());
  }
  if (!FLAGS_repo_subgraph.empty()) {
    repos.ExportRepoSubgraph(FLAGS_repo_subgraph.c_str());
  }
  if (!FLAGS_shout.empty()) {
    if (FLAGS_repo_subgraph.empty() || FLAGS_vnames.empty()) {
      ::fprintf(stderr, "Need the repo subgraph and vnames files.\n");
      return 1;
    }
    repos.ExportIndexScript(FLAGS_shout.c_str(), FLAGS_vnames.c_str(),
                            FLAGS_repo_subgraph.c_str());
  }
  return 0;
}
