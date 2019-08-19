/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#include "absl/memory/memory.h"
#include "absl/strings/str_format.h"
#include "gflags/gflags.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/protobuf_metadata_file.h"
#include "kythe/cxx/indexer/cxx/GoogleFlagsLibrarySupport.h"
#include "kythe/cxx/indexer/cxx/ImputedConstructorSupport.h"
#include "kythe/cxx/indexer/cxx/IndexerFrontendAction.h"
#include "kythe/cxx/indexer/cxx/ProtoLibrarySupport.h"
#include "kythe/cxx/indexer/cxx/frontend.h"
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
DEFINE_bool(experimental_drop_objc_fwd_class_docs, false,
            "Drop comments for Objective-C forward class declarations.");
DEFINE_bool(experimental_drop_cpp_fwd_decl_docs, false,
            "Drop comments for C++ forward declarations.");
DEFINE_int32(experimental_usr_byte_size, 0,
             "Use this many bytes to represent a USR (or don't at all if 0).");

namespace kythe {

int main(int argc, char* argv[]) {
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
  options.ObjCFwdDocs = FLAGS_experimental_drop_objc_fwd_class_docs
                            ? kythe::BehaviorOnFwdDeclComments::Ignore
                            : kythe::BehaviorOnFwdDeclComments::Emit;
  options.CppFwdDocs = FLAGS_experimental_drop_cpp_fwd_decl_docs
                           ? kythe::BehaviorOnFwdDeclComments::Ignore
                           : kythe::BehaviorOnFwdDeclComments::Emit;
  options.UsrByteSize = FLAGS_experimental_usr_byte_size <= 0
                            ? 0
                            : FLAGS_experimental_usr_byte_size;
  options.DropInstantiationIndependentData =
      FLAGS_experimental_drop_instantiation_independent_data;
  options.AllowFSAccess = context.allow_filesystem_access();
  if (FLAGS_report_profiling_events) {
    options.ReportProfileEvent = [](const char* counter, ProfilingEvent event) {
      absl::FPrintF(stderr, "%s: %s\n", counter,
                    event == ProfilingEvent::Enter ? "enter" : "exit");
    };
  }

  bool had_errors = false;
  NullOutputStream null_stream;

  context.EnumerateCompilations([&](IndexerJob& job) {
    options.EffectiveWorkingDirectory = job.unit.working_directory();

    kythe::MetadataSupports meta_supports;
    meta_supports.Add(absl::make_unique<ProtobufMetadataSupport>());
    meta_supports.Add(absl::make_unique<KytheMetadataSupport>());

    kythe::LibrarySupports library_supports;
    library_supports.push_back(absl::make_unique<GoogleFlagsLibrarySupport>());
    library_supports.push_back(absl::make_unique<GoogleProtoLibrarySupport>());
    library_supports.push_back(absl::make_unique<ImputedConstructorSupport>());

    std::string result = IndexCompilationUnit(
        job.unit, job.virtual_files, *context.claim_client(),
        context.hash_cache(),
        job.silent ? static_cast<KytheCachingOutput&>(null_stream)
                   : static_cast<KytheCachingOutput&>(*context.output()),
        options, &meta_supports, &library_supports,
        [](IndexerASTVisitor* indexer) {
          return IndexerWorklist::CreateDefaultWorklist(indexer);
        });

    if (!result.empty()) {
      absl::FPrintF(stderr, "Error: %s\n", result);
      had_errors = true;
    }
  });

  return (had_errors ? 1 : 0);
}

}  // namespace kythe

int main(int argc, char* argv[]) { return kythe::main(argc, argv); }
