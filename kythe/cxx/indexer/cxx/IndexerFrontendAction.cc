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

// This file uses the Clang style conventions.

#include "IndexerFrontendAction.h"

#include <memory>
#include <string>

#include "kythe/cxx/common/indexing/KytheClaimClient.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/indexing/KytheVFS.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/cxx.pb.h"
#include "third_party/llvm/src/clang_builtin_headers.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/Twine.h"

#include "KytheGraphObserver.h"

namespace kythe {

bool RunToolOnCode(std::unique_ptr<clang::FrontendAction> tool_action,
                   llvm::Twine code, const std::string &filename) {
  if (tool_action == nullptr)
    return false;
  return clang::tooling::runToolOnCode(tool_action.release(), code, filename);
}

namespace {

bool DecodeHeaderSearchInformation(const proto::CompilationUnit &Unit,
                                   HeaderSearchInfo &Info) {
  bool FoundDetails = false;
  proto::CxxCompilationUnitDetails Details;
  for (const auto &Any : Unit.details()) {
    if (Any.type_url() == kCxxCompilationUnitDetailsURI) {
      FoundDetails = UnpackAny(Any, &Details);
      break;
    }
  }
  if (!FoundDetails) {
    return false;
  }
  if (!Info.CopyFrom(Details)) {
    fprintf(stderr,
            "Warning: unit has header search info, but it is ill-formed.\n");
    return false;
  }
  return true;
}

std::string ConfigureSystemHeaders(const proto::CompilationUnit &Unit,
                                   std::vector<proto::FileData> &Files) {
  const std::string HeaderPath = "/kythe_builtins/include/";
  for (const auto *Header = builtin_headers_create(); Header->name != nullptr;
       ++Header) {
    auto Path = HeaderPath + Header->name;
    auto Data = Header->data;
    proto::FileData NewFile;
    NewFile.mutable_info()->set_path(Path);
    NewFile.mutable_info()->set_digest("");
    *NewFile.mutable_content() = Data;
    Files.push_back(NewFile);
  }
  return "-resource-dir=/kythe_builtins";
}
} // anonymous namespace

std::string IndexCompilationUnit(const proto::CompilationUnit &Unit,
                                 std::vector<proto::FileData> &Files,
                                 KytheClaimClient &Client, HashCache *Cache,
                                 KytheOutputStream &Output,
                                 const IndexerOptions &Options,
                                 const MetadataSupports *MetaSupports) {
  HeaderSearchInfo HSI;
  bool HSIValid = DecodeHeaderSearchInformation(Unit, HSI);
  std::string FixupArgument;
  if (!HSIValid) {
    FixupArgument = ConfigureSystemHeaders(Unit, Files);
  }
  clang::FileSystemOptions FSO;
  FSO.WorkingDir = Options.EffectiveWorkingDirectory;
  std::vector<llvm::StringRef> Dirs;
  for (auto &Path : HSI.paths) {
    Dirs.push_back(Path.path);
  }
  llvm::IntrusiveRefCntPtr<IndexVFS> VFS(
      new IndexVFS(FSO.WorkingDir, Files, Dirs));
  KytheGraphRecorder Recorder(&Output);
  KytheGraphObserver Observer(&Recorder, &Client, MetaSupports, VFS,
                              Options.ReportProfileEvent);
  if (Cache != nullptr) {
    Output.UseHashCache(Cache);
    Observer.StopDeferringNodes();
  }
  if (Options.DropInstantiationIndependentData) {
    Observer.DropRedundantWraiths();
  }
  Observer.set_claimant(Unit.v_name());
  Observer.set_starting_context(Unit.entry_context());
  Observer.set_lossy_claiming(Options.EnableLossyClaiming);
  for (const auto &Input : Unit.required_input()) {
    if (Input.has_info() && !Input.info().path().empty() &&
        Input.has_v_name()) {
      VFS->SetVName(Input.info().path(), Input.v_name());
    }
    const std::string &FilePath = Input.info().path();
    for (const auto &Row : Input.context()) {
      if (Row.always_process()) {
        auto ClaimableVname = Input.v_name();
        ClaimableVname.set_signature(Row.source_context() +
                                     ClaimableVname.signature());
        Client.AssignClaim(ClaimableVname, Unit.v_name());
      }
      for (const auto &Col : Row.column()) {
        Observer.AddContextInformation(FilePath, Row.source_context(),
                                       Col.offset(), Col.linked_context());
      }
    }
  }
  std::unique_ptr<IndexerFrontendAction> Action(
      new IndexerFrontendAction(&Observer, HSIValid ? &HSI : nullptr));
  Action->setIgnoreUnimplemented(Options.UnimplementedBehavior);
  Action->setTemplateMode(Options.TemplateBehavior);
  Action->setVerbosity(Options.Verbosity);
  llvm::IntrusiveRefCntPtr<clang::FileManager> FileManager(
      new clang::FileManager(FSO, Options.AllowFSAccess ? nullptr : VFS));
  std::vector<std::string> Args(Unit.argument().begin(), Unit.argument().end());
  Args.insert(Args.begin() + 1, "-fsyntax-only");
  Args.insert(Args.begin() + 1, "-w");
  if (!FixupArgument.empty()) {
    Args.insert(Args.begin() + 1, FixupArgument);
  }
  // StdinAdjustSingleFrontendActionFactory takes ownership of its action.
  std::unique_ptr<StdinAdjustSingleFrontendActionFactory> Tool(
      new StdinAdjustSingleFrontendActionFactory(std::move(Action)));
  // ToolInvocation doesn't take ownership of ToolActions.
  clang::tooling::ToolInvocation Invocation(
      Args, Tool.get(), FileManager.get(),
      std::make_shared<clang::PCHContainerOperations>());
  ProfileBlock block(Observer.getProfilingCallback(), "run_invocation");
  if (!Invocation.run()) {
    return "Errors during indexing.";
  }

  return "";
}

} // namespace kythe
