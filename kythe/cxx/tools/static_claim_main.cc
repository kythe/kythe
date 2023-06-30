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
// static_claim: a tool to assign ownership for indexing dependencies
//
// static_claim
//   reads the names of .kzip files from standard input and emits a static claim
//   assignment to standard output

#include <fcntl.h>
#include <sys/stat.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/init.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/re2_flag.h"
#include "kythe/cxx/common/vname_ordering.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/claim.pb.h"
#include "kythe/proto/filecontext.pb.h"

using kythe::proto::ClaimAssignment;
using kythe::proto::CompilationUnit;
using kythe::proto::VName;

ABSL_FLAG(bool, text, false, "Dump output as text instead of protobuf.");
ABSL_FLAG(bool, show_stats, false, "Show some statistics.");
ABSL_FLAG(kythe::RE2Flag, include_files, {},
          "If set, a RE2 pattern of file VName paths to claim.");

struct Claimable;

/// \brief Something (like a compilation unit) that can take responsibility for
/// a claimable object.
struct Claimant {
  /// \brief This Claimant's VName.
  VName vname;
  /// \brief The set of confirmed claims that this Claimant has. Non-owning.
  std::set<Claimable*> claims;
};

/// \brief Stably compares `Claimants` by vname.
struct ClaimantPointerLess {
  bool operator()(const Claimant* lhs, const Claimant* rhs) const {
    return kythe::VNameLess()(lhs->vname, rhs->vname);
  }
};

/// \brief An object (like a header transcript) that a Claimant can take
/// responsibility for.
struct Claimable {
  /// \brief This Claimable's VName.
  VName vname;
  /// \brief Of the `claimants`, which one has responsibility. Non-owning.
  Claimant* elected_claimant;
  /// \brief All of the Claimants that can possibly be given responsibility.
  std::set<Claimant*, ClaimantPointerLess> claimants;
};

/// \brief Populates the compilation units from a kzip.
/// \param path Path to the .kzip file.
/// \return Vector of collected CompilationUnits.
static std::vector<CompilationUnit> ReadCompilationUnits(
    const std::string& path) {
  kythe::IndexReader reader = kythe::KzipReader::Open(path).value();
  std::vector<CompilationUnit> result;
  auto status = reader.Scan([&](const auto digest) {
    const auto compilation = reader.ReadUnit(digest);
    CHECK(compilation.ok()) << compilation.status();
    result.push_back(compilation->unit());
    return true;
  });
  return result;
}

/// \brief Maps from vnames to claimants (like compilation units).
using ClaimantMap = std::map<VName, Claimant, kythe::VNameLess>;

/// \brief Maps from vnames to claimables.
///
/// The vname for a claimable with a transcript (like a header file)
/// is formed from the underlying vname with its signature changed to
/// include the transcript as a prefix.
using ClaimableMap = std::map<VName, Claimable, kythe::VNameLess>;

/// \brief Range wrapper around unpacked ContextDependentVersion rows.
class FileContextRows {
 public:
  using iterator = decltype(
      std::declval<kythe::proto::ContextDependentVersion>().row().begin());

  explicit FileContextRows(
      const kythe::proto::CompilationUnit::FileInput& file_input) {
    for (const google::protobuf::Any& detail : file_input.details()) {
      if (detail.UnpackTo(&context_)) break;
    }
  }

  iterator begin() const { return context_.row().begin(); }
  iterator end() const { return context_.row().end(); }
  bool empty() const { return context_.row().empty(); }

 private:
  kythe::proto::ContextDependentVersion context_;
};

/// \brief Generates and exports a mapping from claimants to claimables.
class ClaimTool {
 public:
  /// \brief Selects a claimant for every claimable.
  ///
  /// We apply a simple heuristic: for every claimable, for every possible
  /// claimant, we choose the claimant with the fewest claimables assigned to
  /// it when trying to assign a new claimable.
  void AssignClaims() {
    // claimables_ is sorted by VName.
    for (auto& claimable : claimables_) {
      CHECK(!claimable.second.claimants.empty());
      Claimant* emptiest_claimant = *claimable.second.claimants.begin();
      // claimants is also sorted by VName, so this assignment should be stable.
      for (auto& claimant : claimable.second.claimants) {
        if (claimant->claims.size() < emptiest_claimant->claims.size()) {
          emptiest_claimant = claimant;
        }
      }
      emptiest_claimant->claims.insert(&claimable.second);
      claimable.second.elected_claimant = emptiest_claimant;
    }
  }

  /// \brief Export claim data to `out_fd` in the format specified by
  /// `FLAGS_text`.
  void WriteClaimFile(int out_fd) {
    if (absl::GetFlag(FLAGS_text)) {
      for (auto& claimable : claimables_) {
        if (claimable.second.elected_claimant) {
          ClaimAssignment claim;
          claim.mutable_compilation_v_name()->CopyFrom(
              claimable.second.elected_claimant->vname);
          claim.mutable_dependency_v_name()->CopyFrom(claimable.second.vname);
          absl::PrintF("%v", claim);
        }
      }
      return;
    }
    {
      namespace io = google::protobuf::io;
      io::FileOutputStream file_output_stream(out_fd);
      io::GzipOutputStream::Options options;
      options.format = io::GzipOutputStream::GZIP;
      io::GzipOutputStream gzip_stream(&file_output_stream, options);
      io::CodedOutputStream coded_stream(&gzip_stream);
      for (auto& claimable : claimables_) {
        const auto& elected_claimant = claimable.second.elected_claimant;
        if (elected_claimant) {
          ClaimAssignment claim;
          claim.mutable_compilation_v_name()->CopyFrom(elected_claimant->vname);
          claim.mutable_dependency_v_name()->CopyFrom(claimable.second.vname);
          coded_stream.WriteVarint32(claim.ByteSizeLong());
          CHECK(claim.SerializeToCodedStream(&coded_stream));
        }
      }
      CHECK(!coded_stream.HadError());
    }
    CHECK(::close(out_fd) == 0) << "errno was: " << errno;
  }

  /// \brief Add `unit` as a possible claimant and remember all of its
  /// dependencies (and their different transcripts) as claimables.
  void HandleCompilationUnit(const CompilationUnit& unit) {
    auto insert_result =
        claimants_.emplace(unit.v_name(), Claimant{unit.v_name()});
    if (!insert_result.second) {
      LOG(WARNING) << "Compilation unit with name " << unit.v_name()
                   << " had the same VName as another previous unit.";
    }
    for (auto& input : unit.required_input()) {
      ++total_input_count_;
      FileContextRows context_rows(input);
      if (!context_rows.empty()) {
        VName input_vname = input.v_name();
        if (!input_vname.signature().empty()) {
          // We generally expect that file vnames have no signature.
          // If this happens, we'll emit a warning, but we'll also be sure to
          // keep the signature around as a suffix when building vnames for
          // contexts.
          LOG(WARNING) << "Input " << input_vname
                       << " has a nonempty signature.\n";
        }
        for (const auto& row : context_rows) {
          // If we have a (r, h, c) entry, we'd better have an input entry for
          // the file included at h with context c (otherwise the index file
          // isn't well-formed). We therefore only need to claim each unique
          // row.
          VName cxt_vname = input_vname;
          cxt_vname.set_signature(row.source_context() +
                                  input_vname.signature());
          Claim(cxt_vname, &insert_result.first->second);
        }
      } else {
        Claim(input.v_name(), &insert_result.first->second);
      }
    }
  }

  const ClaimantMap& claimants() const { return claimants_; }
  const ClaimableMap& claimables() const { return claimables_; }
  size_t total_include_count() const { return total_include_count_; }
  size_t total_input_count() const { return total_input_count_; }
  size_t skipped_input_count() const { return skipped_input_count_; }

 private:
  void Claim(const VName& vname, Claimant* claimant) {
    ++total_include_count_;
    if (auto accept = absl::GetFlag(FLAGS_include_files);
        accept.value != nullptr &&
        !RE2::FullMatch(vname.path(), *accept.value)) {
      ++skipped_input_count_;
      return;
    }

    auto input_insert_result =
        claimables_.emplace(vname, Claimable{vname, nullptr});
    input_insert_result.first->second.claimants.insert(claimant);
  }
  /// Objects that may claim resources.
  ClaimantMap claimants_;
  /// Resources that may be claimed.
  ClaimableMap claimables_;
  /// Number of required inputs.
  size_t total_include_count_ = 0;
  /// Number of #includes.
  size_t total_input_count_ = 0;
  /// Number of required inputs skipped due for failure to match.
  size_t skipped_input_count_ = 0;
};

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  kythe::InitializeProgram(argv[0]);
  absl::SetProgramUsageMessage("static_claim: assign ownership for analysis");
  absl::ParseCommandLine(argc, argv);
  std::string next_index_file;
  ClaimTool tool;
  while (std::getline(std::cin, next_index_file)) {
    if (next_index_file.empty()) {
      continue;
    }
    for (const auto& unit : ReadCompilationUnits(next_index_file)) {
      tool.HandleCompilationUnit(unit);
    }
  }
  if (!std::cin.eof()) {
    absl::FPrintF(stderr, "Error reading from standard input.\n");
    return 1;
  }
  tool.AssignClaims();
  tool.WriteClaimFile(STDOUT_FILENO);
  if (absl::GetFlag(FLAGS_show_stats)) {
    absl::PrintF("Number of claimables: %lu\n", tool.claimables().size());
    absl::PrintF(" Number of claimants: %lu\n", tool.claimants().size());
    absl::PrintF("   Total input count: %lu\n", tool.total_input_count());
    absl::PrintF(" Total include count: %lu\n", tool.total_include_count());
    absl::PrintF(" Skipped input count: %lu\n", tool.skipped_input_count());
    absl::PrintF("%%claimables/includes: %f\n",
                 tool.claimables().size() * 100.0 / tool.total_include_count());
  }
  return 0;
}
