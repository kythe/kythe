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

#include <stdio.h>
#include <unistd.h>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"

#include "kythe/proto/storage.pb.h"

#include "assertion_ast.h"
#include "verifier.h"

DEFINE_bool(show_protos, false, "Show protocol buffers read from standard in");
DEFINE_bool(show_goals, false, "Show goals after parsing");
DEFINE_bool(ignore_dups, false, "Ignore duplicate facts during verification");
DEFINE_bool(graphviz, false, "Only dump facts as a GraphViz-compatible graph");
DEFINE_bool(annotated_graphviz, false, "Solve and annotate a GraphViz graph.");
DEFINE_string(goal_prefix, "//-", "Denote goals with this string.");

int main(int argc, char **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  ::google::SetVersionString("0.1");
  ::google::SetUsageMessage(R"(Verification tool for Kythe databases.
Reads Kythe facts from standard input and checks them against one or more rule
files. See the DESIGN file for more details on invocation and rule syntax.

Example:
  ${INDEXER_BIN} -i $1 | ${VERIFIER_BIN} --show_protos --show_goals $1
  cat foo.entries | ${VERIFIER_BIN} goals1.cc goals2.cc
)");
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);

  kythe::verifier::Verifier v;
  v.set_goal_comment_marker(FLAGS_goal_prefix);

  if (FLAGS_ignore_dups) {
    v.IgnoreDuplicateFacts();
  }

  if (FLAGS_annotated_graphviz) {
    v.SaveEVarAssignments();
  }

  if (!FLAGS_graphviz) {
    std::vector<std::string> rule_files(argv + 1, argv + argc);
    if (rule_files.empty()) {
      fprintf(stderr, "No rule files specified\n");
      return 1;
    }

    for (const auto &rule_file : rule_files) {
      if (rule_file.empty()) {
        continue;
      }
      if (!v.LoadInlineRuleFile(rule_file)) {
        fprintf(stderr, "Failed loading %s.\n", rule_file.c_str());
        return 2;
      }
    }
  }

  std::string dbname = "database";
  size_t facts = 0;
  kythe::proto::Entry entry;
  google::protobuf::uint32 byte_size;
  google::protobuf::io::FileInputStream raw_input(STDIN_FILENO);
  google::protobuf::io::CodedInputStream coded_input(&raw_input);
  coded_input.SetTotalBytesLimit(INT_MAX, -1);
  while (coded_input.ReadVarint32(&byte_size)) {
    auto limit = coded_input.PushLimit(byte_size);
    if (!entry.ParseFromCodedStream(&coded_input)) {
      fprintf(stderr, "Error reading around fact %zu\n", facts);
      return 1;
    }
    coded_input.PopLimit(limit);
    if (FLAGS_show_protos) {
      entry.PrintDebugString();
    }
    v.AssertSingleFact(&dbname, facts, entry);
    ++facts;
  }

  if (FLAGS_show_goals) {
    v.ShowGoals();
  }

  int result = 0;

  if (!v.VerifyAllGoals()) {
    fprintf(stderr,
            "Could not verify all goals. The furthest we reached was:\n  ");
    v.DumpErrorGoal(v.highest_group_reached(), v.highest_goal_reached());
    result = 1;
  }

  if (FLAGS_graphviz || FLAGS_annotated_graphviz) {
    v.DumpAsDot();
  }

  return result;
}
