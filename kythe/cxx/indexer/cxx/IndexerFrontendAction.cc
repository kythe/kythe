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

#include "IndexerFrontendAction.h"

#include <memory>
#include <string>
#include <utility>

#include "KytheGraphObserver.h"
#include "KytheVFS.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/indexer/cxx/KytheClaimClient.h"
#include "kythe/cxx/indexer/cxx/KytheVFS.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/buildinfo.pb.h"
#include "kythe/proto/cxx.pb.h"
#include "kythe/proto/filecontext.pb.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "third_party/llvm/src/clang_builtin_headers.h"

namespace kythe {

bool RunToolOnCode(std::unique_ptr<clang::FrontendAction> tool_action,
                   llvm::Twine code, const std::string& filename) {
  if (tool_action == nullptr) return false;
  return clang::tooling::runToolOnCode(std::move(tool_action), code, filename);
}

namespace {

// Message type URI for the build details message.
constexpr absl::string_view kBuildDetailsURI =
    "kythe.io/proto/kythe.proto.BuildDetails";

/// \brief Range wrapper around unpacked ContextDependentVersion rows.
class FileContextRows {
 public:
  using iterator =
      decltype(std::declval<kythe::proto::ContextDependentVersion>()
                   .row()
                   .begin());

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

bool DecodeDetails(const proto::CompilationUnit& Unit,
                   proto::CxxCompilationUnitDetails& Details) {
  for (const auto& Any : Unit.details()) {
    if (Any.type_url() == kCxxCompilationUnitDetailsURI) {
      if (UnpackAny(Any, &Details)) {
        return true;
      }
    }
  }
  return false;
}

std::string ExtractBuildConfig(const proto::CompilationUnit& Unit) {
  proto::BuildDetails details;
  for (const auto& Any : Unit.details()) {
    if (Any.type_url() == kBuildDetailsURI) {
      if (UnpackAny(Any, &details)) {
        return details.build_config();
      }
    }
  }
  return "";
}

bool DecodeHeaderSearchInfo(const proto::CxxCompilationUnitDetails& Details,
                            HeaderSearchInfo& Info) {
  if (!Details.has_header_search_info()) {
    return false;
  }
  if (!Info.CopyFrom(Details)) {
    absl::FPrintF(
        stderr,
        "Warning: unit has header search info, but it is ill-formed.\n");
    return false;
  }
  return true;
}

std::string ConfigureSystemHeaders(const proto::CompilationUnit& Unit,
                                   std::vector<proto::FileData>& Files) {
  std::vector<proto::FileData> OldFiles;
  OldFiles.swap(Files);
  const std::string HeaderPath = "/kythe_builtins/include/";
  std::unordered_set<std::string> NewHeaders;
  for (const auto* Header = builtin_headers_create(); Header->name != nullptr;
       ++Header) {
    auto Path = HeaderPath + Header->name;
    auto Data = Header->data;
    proto::FileData NewFile;
    NewFile.mutable_info()->set_path(Path);
    NewFile.mutable_info()->set_digest("");
    *NewFile.mutable_content() = Data;
    Files.push_back(NewFile);
    NewHeaders.insert(Path);
  }
  for (const auto& File : OldFiles) {
    if (NewHeaders.find(File.info().path()) == NewHeaders.end()) {
      Files.push_back(File);
    }
  }
  return "-resource-dir=/kythe_builtins";
}

std::string FormatLocation(clang::FullSourceLoc loc) {
  if (!loc.isValid()) {
    return "";
  }
  return absl::StrCat(loc.printToString(loc.getManager()), ": ");
}

// Collects text-formatted errors for later.
class TextErrorBuffer : public clang::DiagnosticConsumer {
 public:
  void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                        const clang::Diagnostic& info) override {
    DiagnosticConsumer::HandleDiagnostic(level, info);

    llvm::SmallString<100> buf;
    info.FormatDiagnostic(buf);
    switch (level) {
      case clang::DiagnosticsEngine::Error:
      case clang::DiagnosticsEngine::Fatal:
        errors_.push_back(
            absl::StrCat(FormatLocation(clang::FullSourceLoc(
                             info.getLocation(), info.getSourceManager())),
                         buf.c_str()));
        break;
      default:
        break;
    }
  }

  const std::vector<std::string>& errors() const { return errors_; }

 private:
  std::vector<std::string> errors_;
};
}  // anonymous namespace

std::string IndexCompilationUnit(
    const proto::CompilationUnit& Unit, std::vector<proto::FileData>& Files,
    KytheClaimClient& Client, HashCache* Cache, KytheCachingOutput& Output,
    IndexerOptions& Options ABSL_ATTRIBUTE_LIFETIME_BOUND,
    const MetadataSupports* MetaSupports,
    const LibrarySupports* LibrarySupports) {
  llvm::sys::path::Style Style =
      kythe::IndexVFS::DetectStyleFromAbsoluteWorkingDirectory(
          Unit.working_directory())
          .value_or(llvm::sys::path::Style::posix);
  HeaderSearchInfo HSI;
  proto::CxxCompilationUnitDetails Details;
  bool HSIValid = false;
  std::vector<llvm::StringRef> Dirs;
  if (DecodeDetails(Unit, Details)) {
    HSIValid = DecodeHeaderSearchInfo(Details, HSI);
    for (const auto& stat_path : Details.stat_path()) {
      Dirs.push_back(stat_path.path());
    }
  }
  std::string FixupArgument = ConfigureSystemHeaders(Unit, Files);
  if (HSIValid) {
    FixupArgument.clear();
  }
  clang::FileSystemOptions FSO;
  FSO.WorkingDir = Options.EffectiveWorkingDirectory;
  if (Style == llvm::sys::path::Style::windows) {
    FSO.WorkingDir = absl::StrCat("/", FSO.WorkingDir);
  }
  for (auto& Path : HSI.paths) {
    Dirs.push_back(Path.path);
  }
  llvm::IntrusiveRefCntPtr<IndexVFS> VFS(
      new IndexVFS(Options.EffectiveWorkingDirectory, Files, Dirs, Style));
  KytheGraphRecorder Recorder(&Output);
  KytheGraphObserverOptions options;
  options.build_config = ExtractBuildConfig(Unit);
  options.default_corpus =
      Options.UseCompilationCorpusAsDefault ? Unit.v_name().corpus() : "";
  options.usr_default_corpus = Options.EmitUsrCorpus;
  options.hash_recorder = Options.HashRecorder;
  KytheGraphObserver Observer(&Recorder, &Client, MetaSupports, VFS,
                              Options.ReportProfileEvent, options);
  if (Cache != nullptr) {
    Output.UseHashCache(Cache);
    Observer.StopDeferringNodes();
  }
  if (Options.DropInstantiationIndependentData) {
    Observer.DropRedundantWraiths();
  }
  Observer.set_claimant(Unit.v_name());
  Observer.set_starting_context(Unit.entry_context());
  for (const auto& Input : Unit.required_input()) {
    if (Input.has_info() && !Input.info().path().empty() &&
        Input.has_v_name()) {
      VFS->SetVName(Input.info().path(), Input.v_name());
    }
    const std::string& FilePath = Input.info().path();
    for (const auto& Row : FileContextRows(Input)) {
      if (Row.always_process()) {
        auto ClaimableVname = Input.v_name();
        ClaimableVname.set_signature(Row.source_context() +
                                     ClaimableVname.signature());
        Client.AssignClaim(ClaimableVname, Unit.v_name());
      }
      for (const auto& Col : Row.column()) {
        Observer.AddContextInformation(FilePath, Row.source_context(),
                                       Col.offset(), Col.linked_context());
      }
    }
  }
  if (MetaSupports != nullptr) {
    MetaSupports->UseVNameLookup(
        [VFS](const std::string& path, proto::VName* out) {
          return VFS->get_vname(path, out);
        });
  }
  std::unique_ptr<IndexerFrontendAction> Action =
      std::make_unique<IndexerFrontendAction>(
          &Observer, HSIValid ? &HSI : nullptr, LibrarySupports, Options);
  llvm::IntrusiveRefCntPtr<clang::FileManager> FileManager(
      new clang::FileManager(FSO, Options.AllowFSAccess ? nullptr : VFS));
  std::vector<std::string> Args(Unit.argument().begin(), Unit.argument().end());
  Args.insert(Args.begin() + 1, {"-nocudalib", "-w", "-fsyntax-only"});
  if (!FixupArgument.empty()) {
    Args.insert(Args.begin() + 1, FixupArgument);
  }

  // StdinAdjustSingleFrontendActionFactory takes ownership of its action.
  std::unique_ptr<StdinAdjustSingleFrontendActionFactory> Tool =
      std::make_unique<StdinAdjustSingleFrontendActionFactory>(
          std::move(Action));
  // ToolInvocation doesn't take ownership of ToolActions.
  clang::tooling::ToolInvocation Invocation(
      Args, Tool.get(), FileManager.get(),
      std::make_shared<clang::PCHContainerOperations>());

  TextErrorBuffer Diags;
  Invocation.setDiagnosticConsumer(&Diags);

  ProfileBlock block(Observer.getProfilingCallback(), "run_invocation");
  if (!Invocation.run()) {
    return absl::StrCat("Errors during indexing:",
                        absl::StrJoin(Diags.errors(), "\n"));
  }
  return "";
}

}  // namespace kythe
