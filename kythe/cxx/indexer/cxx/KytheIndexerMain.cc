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
//       indexer some/index.kzip

#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/str_format.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/protobuf_metadata_file.h"
#include "kythe/cxx/common/re2_flag.h"
#include "kythe/cxx/indexer/cxx/GoogleFlagsLibrarySupport.h"
#include "kythe/cxx/indexer/cxx/ImputedConstructorSupport.h"
#include "kythe/cxx/indexer/cxx/IndexerFrontendAction.h"
#include "kythe/cxx/indexer/cxx/ProtoLibrarySupport.h"
#include "kythe/cxx/indexer/cxx/frontend.h"
#include "kythe/cxx/indexer/cxx/indexer_worklist.h"

ABSL_FLAG(bool, index_template_instantiations, true,
          "Index template instantiations.");
ABSL_FLAG(bool, experimental_drop_instantiation_independent_data, false,
          "Don't emit template nodes and edges found to be "
          "instantiation-independent.");
ABSL_FLAG(bool, report_profiling_events, false,
          "Write profiling events to standard error.");
ABSL_FLAG(bool, experimental_drop_objc_fwd_class_docs, true,
          "Drop comments for Objective-C forward class declarations.");
ABSL_FLAG(bool, experimental_drop_cpp_fwd_decl_docs, true,
          "Drop comments for C++ forward declarations.");
ABSL_FLAG(int, experimental_usr_byte_size, 0,
          "Use this many bytes to represent a USR (or don't at all if 0).");
ABSL_FLAG(bool, use_compilation_corpus_as_default, true,
          "Use the CompilationUnit VName corpus as the default.");
ABSL_FLAG(kythe::RE2Flag, template_instance_exclude_path_pattern,
          kythe::RE2Flag{},
          "If nonempty, a regex that matches files to be excluded from "
          "template instance indexing.");
ABSL_FLAG(std::string, record_hashes_file, "", "Record hashes to this file.");
ABSL_FLAG(bool, record_call_directness, false,
          "Record directness of function calls.");
ABSL_FLAG(bool, emit_usr_corpus, false,
          "Use the default corpus when emitting USR nodes.");
ABSL_FLAG(bool, experimental_set_aliases_as_writes, false,
          "Set protobuf aliases as writes.");

namespace kythe {

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(
      IndexerContext::UsageMessage("the Kythe C++ indexer", "indexer"));
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  std::vector<std::string> final_args(remain.begin(), remain.end());
  IndexerContext context(final_args, "stdin.cc");
  IndexerOptions options;
  options.TemplateMode = absl::GetFlag(FLAGS_index_template_instantiations)
                             ? BehaviorOnTemplates::VisitInstantiations
                             : BehaviorOnTemplates::SkipInstantiations;
  options.IgnoreUnimplemented = context.ignore_unimplemented()
                                    ? kythe::BehaviorOnUnimplemented::Continue
                                    : kythe::BehaviorOnUnimplemented::Abort;
  options.ObjCFwdDocs =
      absl::GetFlag(FLAGS_experimental_drop_objc_fwd_class_docs)
          ? kythe::BehaviorOnFwdDeclComments::Ignore
          : kythe::BehaviorOnFwdDeclComments::Emit;
  options.CppFwdDocs = absl::GetFlag(FLAGS_experimental_drop_cpp_fwd_decl_docs)
                           ? kythe::BehaviorOnFwdDeclComments::Ignore
                           : kythe::BehaviorOnFwdDeclComments::Emit;
  options.UsrByteSize = absl::GetFlag(FLAGS_experimental_usr_byte_size) <= 0
                            ? 0
                            : absl::GetFlag(FLAGS_experimental_usr_byte_size);
  options.EmitUsrCorpus = absl::GetFlag(FLAGS_emit_usr_corpus);
  options.TemplateInstanceExcludePathPattern =
      absl::GetFlag(FLAGS_template_instance_exclude_path_pattern).value;
  options.UseCompilationCorpusAsDefault =
      absl::GetFlag(FLAGS_use_compilation_corpus_as_default);
  options.DropInstantiationIndependentData =
      absl::GetFlag(FLAGS_experimental_drop_instantiation_independent_data);
  options.AllowFSAccess = context.allow_filesystem_access();
  if (absl::GetFlag(FLAGS_report_profiling_events)) {
    options.ReportProfileEvent = [](const char* counter, ProfilingEvent event) {
      absl::FPrintF(stderr, "%s: %s\n", counter,
                    event == ProfilingEvent::Enter ? "enter" : "exit");
    };
  }
  options.RecordCallDirectness = absl::GetFlag(FLAGS_record_call_directness);
  options.CreateWorklist = [](IndexerASTVisitor* indexer) {
    return IndexerWorklist::CreateDefaultWorklist(indexer);
  };
  std::optional<FileHashRecorder> recorder;
  if (!absl::GetFlag(FLAGS_record_hashes_file).empty()) {
    recorder.emplace(absl::GetFlag(FLAGS_record_hashes_file));
    options.HashRecorder = &*recorder;
  }

  bool had_errors = false;
  NullOutputStream null_stream;

  context.EnumerateCompilations([&](IndexerJob& job) {
    options.EffectiveWorkingDirectory = job.unit.working_directory();

    kythe::MetadataSupports meta_supports;
    auto proto = std::make_unique<ProtobufMetadataSupport>();
    proto->SetAliasesAsWrites(
        absl::GetFlag(FLAGS_experimental_set_aliases_as_writes));
    meta_supports.Add(std::move(proto));
    meta_supports.Add(std::make_unique<KytheMetadataSupport>());

    kythe::LibrarySupports library_supports;
    library_supports.push_back(std::make_unique<GoogleFlagsLibrarySupport>());
    library_supports.push_back(std::make_unique<GoogleProtoLibrarySupport>());
    library_supports.push_back(std::make_unique<ImputedConstructorSupport>());

    std::string result = IndexCompilationUnit(
        job.unit, job.virtual_files, *context.claim_client(),
        context.hash_cache(),
        job.silent ? static_cast<KytheCachingOutput&>(null_stream)
                   : static_cast<KytheCachingOutput&>(*context.output()),
        options, &meta_supports, &library_supports);

    if (!result.empty()) {
      absl::FPrintF(stderr, "Error: %s\n", result);
      had_errors = true;
    }
  });

  return (had_errors ? 1 : 0);
}

}  // namespace kythe

int main(int argc, char* argv[]) { return kythe::main(argc, argv); }
