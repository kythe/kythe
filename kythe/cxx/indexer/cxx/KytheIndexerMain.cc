/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Allows the Kythe C++ indexer to be invoked from the command line. By default,
// this program reads a single C++ compilation unit from stdin and emits
// binary Kythe artifacts to stdout as a sequence of Entity protos.
// Command-line arguments may be passed to Clang as positional parameters.
//
//   eg: indexer -i foo.cc -o foo.bin -- -DINDEXING
//       indexer -i foo.cc | verifier foo.cc
//       indexer some/index.kindex

#include "gflags/gflags.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/indexing/frontend.h"
#include "kythe/cxx/common/protobuf_metadata_file.h"
#include "kythe/cxx/indexer/cxx/IndexerFrontendAction.h"
#include "kythe/cxx/indexer/cxx/indexer_worklist.h"

DEFINE_bool(index_template_instantiations, true,
            "Index template instantiations.");
DEFINE_bool(experimental_drop_instantiation_independent_data, false,
            "Don't emit template nodes and edges found to be "
            "instantiation-independent.");
DEFINE_bool(report_profiling_events, false,
            "Write profiling events to standard error.");
DEFINE_bool(experimental_index_lite, false,
            "Drop uncommonly-used data from the index.");

namespace kythe {

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  gflags::SetVersionString("0.1");
  gflags::SetUsageMessage(
      IndexerContext::UsageMessage("the Kythe C++ indexer", "indexer"));
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::vector<std::string> final_args(argv, argv + argc);
  IndexerContext context(final_args, "stdin.cc");
  IndexerOptions options;
  options.TemplateBehavior = FLAGS_index_template_instantiations
                                 ? BehaviorOnTemplates::VisitInstantiations
                                 : BehaviorOnTemplates::SkipInstantiations;
  options.UnimplementedBehavior = context.ignore_unimplemented()
                                      ? kythe::BehaviorOnUnimplemented::Continue
                                      : kythe::BehaviorOnUnimplemented::Abort;
  options.Verbosity = FLAGS_experimental_index_lite ? kythe::Verbosity::Lite
                                                    : kythe::Verbosity::Classic;
  options.DropInstantiationIndependentData =
      FLAGS_experimental_drop_instantiation_independent_data;
  options.AllowFSAccess = context.allow_filesystem_access();
  if (FLAGS_report_profiling_events) {
    options.ReportProfileEvent = [](const char *counter, ProfilingEvent event) {
      fprintf(stderr, "%s: %s\n", counter,
              event == ProfilingEvent::Enter ? "enter" : "exit");
    };
  }

  bool had_errors = false;
  NullOutputStream null_stream;

  for (auto &job : *context.jobs()) {
    options.EffectiveWorkingDirectory = job.working_directory;

    kythe::MetadataSupports meta_supports;
    meta_supports.Add(llvm::make_unique<ProtobufMetadataSupport>());
    meta_supports.Add(llvm::make_unique<KytheMetadataSupport>());

    std::string result = IndexCompilationUnit(
        job.unit, job.virtual_files, *context.claim_client(),
        context.hash_cache(),
        job.silent ? static_cast<KytheOutputStream &>(null_stream)
                   : static_cast<KytheOutputStream &>(*context.output()),
        options, &meta_supports, [](IndexerASTVisitor *indexer) {
          return IndexerWorklist::CreateDefaultWorklist(indexer);
        });

    if (!result.empty()) {
      fprintf(stderr, "Error: %s\n", result.c_str());
      had_errors = true;
    }
  }

  return (had_errors ? 1 : 0);
}

}  // namespace kythe

int main(int argc, char *argv[]) { return kythe::main(argc, argv); }
