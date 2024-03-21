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

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "assertion_ast.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/init.h"
#include "kythe/proto/storage.pb.h"
#include "verifier.h"

ABSL_FLAG(bool, show_protos, false,
          "Show protocol buffers read from standard in");
ABSL_FLAG(bool, show_goals, false, "Show goals after parsing");
ABSL_FLAG(bool, ignore_dups, false,
          "Ignore duplicate facts during verification");
ABSL_FLAG(bool, ignore_code_conflicts, false,
          "Ignore conflicting /kythe/code facts during verification");
ABSL_FLAG(bool, graphviz, false,
          "Only dump facts as a GraphViz-compatible graph");
ABSL_FLAG(bool, annotated_graphviz, false,
          "Solve and annotate a GraphViz graph.");
ABSL_FLAG(bool, minimal_graphviz, false,
          "Solve and dump a GraphViz graph eliding unused nodes.");
ABSL_FLAG(std::string, goal_prefix, "//-", "Denote goals with this string.");
ABSL_FLAG(bool, use_file_nodes, false,
          "Look for assertions in UTF8 file nodes.");
ABSL_FLAG(bool, check_for_singletons, true, "Fail on singleton variables.");
ABSL_FLAG(std::string, goal_regex, "",
          "If nonempty, denote goals with this regex. "
          "The regex must match the entire line. Expects one capture group.");
ABSL_FLAG(bool, convert_marked_source, false,
          "Convert MarkedSource-valued facts to subgraphs.");
ABSL_FLAG(bool, show_anchors, false, "Show anchor locations instead of @s");
ABSL_FLAG(bool, show_vnames, true,
          "Show VNames for nodes which also have labels.");
ABSL_FLAG(bool, show_fact_prefix, true,
          "Include the /kythe or /kythe/edge prefix on facts and edges.");
ABSL_FLAG(bool, file_vnames, true,
          "Find file vnames by matching file content.");
ABSL_FLAG(bool, allow_missing_file_vnames, false,
          "If file_vnames is set, treat missing file vnames as non-fatal.");
ABSL_FLAG(bool, verbose, false,
          "If verbose is set, more logging will be emitted.");
ABSL_FLAG(bool, use_fast_solver, true, "Use the fast solver.");
ABSL_FLAG(bool, print_timing_information, false,
          "Print timing information for profiling.");
ABSL_FLAG(std::string, default_file_corpus, "",
          "Use this corpus for anchor goals for file nodes without a corpus "
          "set. In the future, if this flag is left empty, these file nodes "
          "will raise an error.");

namespace {
// The fast solver needs extra stack space for modest programs.
constexpr rlim_t kSolverStack = 64L * 1024L * 1024L;
}  // namespace

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage(R"(Verification tool for Kythe databases.
Reads Kythe facts from standard input and checks them against one or more rule
files. See https://kythe.io/docs/kythe-verifier.html for more details on
invocation and rule syntax.

Example:
  ${INDEXER_BIN} -i $1 | ${VERIFIER_BIN} --show_protos --show_goals $1
  cat foo.entries | ${VERIFIER_BIN} goals1.cc goals2.cc
  cat foo.entries | ${VERIFIER_BIN} --use_file_nodes
)");
  auto start_time = absl::Now();
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);

  kythe::verifier::Verifier v;
  if (absl::GetFlag(FLAGS_goal_regex).empty()) {
    v.SetGoalCommentPrefix(absl::GetFlag(FLAGS_goal_prefix));
  } else {
    std::string error;
    if (!v.SetGoalCommentRegex(absl::GetFlag(FLAGS_goal_regex), &error)) {
      absl::FPrintF(stderr, "While parsing goal regex: %s\n", error);
      return 1;
    }
  }

  if (absl::GetFlag(FLAGS_ignore_dups)) {
    v.IgnoreDuplicateFacts();
  }

  if (absl::GetFlag(FLAGS_ignore_code_conflicts)) {
    v.IgnoreCodeConflicts();
  }

  if (absl::GetFlag(FLAGS_annotated_graphviz)) {
    v.SaveEVarAssignments();
  }

  if (absl::GetFlag(FLAGS_minimal_graphviz)) {
    v.SaveEVarAssignments();
    v.ElideUnlabeled();
  }

  if (absl::GetFlag(FLAGS_use_file_nodes)) {
    v.UseFileNodes();
  }

  if (absl::GetFlag(FLAGS_convert_marked_source)) {
    v.ConvertMarkedSource();
  }

  if (absl::GetFlag(FLAGS_show_anchors)) {
    v.ShowAnchors();
  }

  if (absl::GetFlag(FLAGS_show_vnames)) {
    v.ShowLabeledVnames();
  }

  if (!absl::GetFlag(FLAGS_file_vnames)) {
    v.IgnoreFileVnames();
  }

  if (absl::GetFlag(FLAGS_allow_missing_file_vnames)) {
    if (!absl::GetFlag(FLAGS_file_vnames)) {
      fprintf(stderr, "--allow_missing_file_vnames needs --file_vnames\n");
      return 1;
    }
    v.AllowMissingFileVNames();
  }

  if (absl::GetFlag(FLAGS_verbose)) {
    v.Verbose();
  }

  if (absl::GetFlag(FLAGS_show_fact_prefix)) {
    v.ShowFactPrefix();
  }

  if (!absl::GetFlag(FLAGS_default_file_corpus).empty()) {
    v.UseDefaultFileCorpus(absl::GetFlag(FLAGS_default_file_corpus));
  }

  v.UseFastSolver(absl::GetFlag(FLAGS_use_fast_solver));

  if (absl::GetFlag(FLAGS_use_fast_solver)) {
    struct rlimit rl;
    int r = getrlimit(RLIMIT_STACK, &rl);
    if (r != 0) {
      perror("failed getting solver stack size");
      return 1;
    }
    if (rl.rlim_cur < kSolverStack) {
      rl.rlim_cur = kSolverStack;
      r = setrlimit(RLIMIT_STACK, &rl);
      if (r != 0) {
        perror("failed increasing solver stack size");
        return 1;
      }
    }
  }

  std::string dbname = "database";
  size_t facts = 0;
  auto start_read_time = absl::Now();
  kythe::proto::Entry entry;
  uint32_t byte_size;
  google::protobuf::io::FileInputStream raw_input(STDIN_FILENO);
  for (;;) {
    google::protobuf::io::CodedInputStream coded_input(&raw_input);
    coded_input.SetTotalBytesLimit(INT_MAX);
    if (!coded_input.ReadVarint32(&byte_size)) {
      break;
    }
    auto limit = coded_input.PushLimit(byte_size);
    if (!entry.ParseFromCodedStream(&coded_input)) {
      absl::FPrintF(stderr, "Error reading around fact %zu\n", facts);
      return 1;
    }
    if (absl::GetFlag(FLAGS_show_protos)) {
      entry.PrintDebugString();
      putchar('\n');
    }
    if (!v.AssertSingleFact(&dbname, facts, entry)) {
      absl::FPrintF(stderr, "Error asserting fact %zu\n", facts);
      return 1;
    }
    ++facts;
  }
  auto done_read_time = absl::Now();

  if (!absl::GetFlag(FLAGS_use_fast_solver) && !v.PrepareDatabase()) {
    return 1;
  }
  auto done_db_time = absl::Now();

  if (!absl::GetFlag(FLAGS_graphviz)) {
    std::vector<std::string> rule_files(remain.begin() + 1, remain.end());
    if (rule_files.empty() && !absl::GetFlag(FLAGS_use_file_nodes)) {
      absl::FPrintF(stderr, "No rule files specified\n");
      return 1;
    }

    for (const auto& rule_file : rule_files) {
      if (rule_file.empty()) {
        continue;
      }
      if (!v.LoadInlineRuleFile(rule_file)) {
        absl::FPrintF(stderr, "Failed loading %s.\n", rule_file);
        return 2;
      }
    }
  }

  if (absl::GetFlag(FLAGS_check_for_singletons) && v.CheckForSingletonEVars()) {
    return 1;
  }

  if (absl::GetFlag(FLAGS_show_goals)) {
    v.ShowGoals();
  }

  int result = 0;

  auto pre_solve_time = absl::Now();
  if (!v.VerifyAllGoals()) {
    // Flush stdout in case any data is stuck in buffers, to avoid mangling a
    // joined output stream.
    fflush(stdout);
    absl::FPrintF(
        stderr, "Could not verify all goals. The furthest we reached was:\n  ");
    v.DumpErrorGoal(v.highest_group_reached(), v.highest_goal_reached());
    result = 1;
  }
  auto post_solve_time = absl::Now();

  if (absl::GetFlag(FLAGS_graphviz) ||
      absl::GetFlag(FLAGS_annotated_graphviz) ||
      absl::GetFlag(FLAGS_minimal_graphviz)) {
    v.DumpAsDot();
  }

  if (absl::GetFlag(FLAGS_print_timing_information)) {
    auto print_timer = [](absl::Duration duration, absl::string_view name) {
      absl::FPrintF(stderr, "%s took %ld msec\n", name,
                    absl::ToInt64Milliseconds(duration));
    };
    print_timer(start_read_time - start_time, "startup");
    print_timer(done_read_time - start_read_time, "read input stream");
    print_timer(done_db_time - done_read_time, "database preparation");
    print_timer(pre_solve_time - done_db_time, "other prep");
    print_timer(post_solve_time - pre_solve_time, "solve");
    print_timer(post_solve_time - start_time, "total elapsed");
  }

  return result;
}
