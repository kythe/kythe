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

// cxx_extractor is meant to be a drop-in replacement for clang/gcc's frontend.
// It collects all of the resources that clang would use to compile a single
// source file (as determined by the command line arguments) and produces a
// .kindex file.
//
// We read environment variables KYTHE_CORPUS (to set the default corpus),
// KYTHE_ROOT_DIRECTORY (to set the default root directory and to configure
// Clang's header search), KYTHE_OUTPUT_DIRECTORY (to control where kindex
// files are deposited), and KYTHE_VNAMES (to control vname generation).
//
// If KYTHE_INDEX_PACK is set to "1", the extractor will treat
// KYTHE_OUTPUT_DIRECTORY as an index pack. Instead of emitting kindex files,
// it will instead follow the index pack protocol.
//
// If the first two arguments are --with_executable /foo/bar, the extractor
// will consider /foo/bar to be the executable it was called as for purposes
// of argument interpretation. These arguments are then stripped.

// If -resource-dir (a Clang argument) is *not* provided, versions of the
// compiler header files embedded into the extractor's executable will be
// mapped to /kythe_builtins and used.

#include "cxx_extractor.h"

#include "clang/Basic/Version.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/cxx/common/CommandLineUtils.h"
#include "llvm/Support/Path.h"

/// When a -resource-dir is not specified, map builtin versions of compiler
/// headers to this directory.
constexpr char kBuiltinResourceDirectory[] = "/kythe_builtins";

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetVersionString("0.1");
  std::vector<std::string> final_args(argv, argv + argc);
  std::string actual_executable = final_args.size() ? final_args[0] : "";
  if (final_args.size() >= 3 && final_args[1] == "--with_executable") {
    final_args.assign(final_args.begin() + 2, final_args.end());
  }
  final_args = kythe::common::GCCArgsToClangSyntaxOnlyArgs(final_args);
  kythe::IndexWriter index_writer;
  // Check to see if an alternate resource-dir was specified; otherwise,
  // invent one. We need this to find stddef.h and friends.
  bool map_builtin_resources = true;
  for (const auto& arg : final_args) {
    // Handle both -resource-dir=foo and -resource-dir foo.
    if (llvm::StringRef(arg).startswith("-resource-dir")) {
      map_builtin_resources = false;
      break;
    }
  }
  if (map_builtin_resources) {
    final_args.insert(final_args.begin() + 1, kBuiltinResourceDirectory);
    final_args.insert(final_args.begin() + 1, "-resource-dir");
  }
  // Store the arguments post-filtering.
  index_writer.set_args(final_args);
  clang::FileSystemOptions file_system_options;
  if (const char* env_corpus = getenv("KYTHE_CORPUS")) {
    index_writer.set_corpus(env_corpus);
  }
  if (const char* vname_file = getenv("KYTHE_VNAMES")) {
    FILE* vname_handle = fopen(vname_file, "rb");
    CHECK(vname_handle != nullptr) << "Couldn't open input vnames file "
                                   << vname_file;
    CHECK_EQ(fseek(vname_handle, 0, SEEK_END), 0) << "Couldn't seek "
                                                  << vname_file;
    long vname_size = ftell(vname_handle);
    CHECK_GE(vname_size, 0) << "Bad size for " << vname_file;
    CHECK_EQ(fseek(vname_handle, 0, SEEK_SET), 0) << "Couldn't seek "
                                                  << vname_file;
    std::string vname_content;
    vname_content.resize(vname_size);
    CHECK_EQ(fread(&vname_content[0], vname_size, 1, vname_handle), 1)
        << "Couldn't read " << vname_file;
    CHECK_NE(fclose(vname_handle), EOF) << "Couldn't close " << vname_file;
    if (!index_writer.SetVNameConfiguration(vname_content)) {
      fprintf(stderr, "Couldn't configure vnames from %s\n", vname_file);
      exit(1);
    }
  }
  if (const char* env_root_directory = getenv("KYTHE_ROOT_DIRECTORY")) {
    index_writer.set_root_directory(env_root_directory);
    file_system_options.WorkingDir = env_root_directory;
  }
  bool using_index_packs = false;
  if (const char* env_index_pack = getenv("KYTHE_INDEX_PACK")) {
    using_index_packs = env_index_pack == std::string("1");
  }
  if (const char* env_output_directory = getenv("KYTHE_OUTPUT_DIRECTORY")) {
    index_writer.set_output_directory(env_output_directory);
  }
  {
    llvm::IntrusiveRefCntPtr<clang::FileManager> file_manager(
        new clang::FileManager(file_system_options));
    auto extractor = kythe::NewExtractor(
        &index_writer,
        [&index_writer, &using_index_packs](
            const std::string& main_source_file,
            const kythe::PreprocessorTranscript& transcript,
            const std::unordered_map<std::string, kythe::SourceFile>&
                source_files,
            const kythe::HeaderSearchInfo& header_search_info,
            bool had_errors) {
          std::unique_ptr<kythe::IndexWriterSink> sink;
          if (using_index_packs) {
            sink.reset(new kythe::IndexPackWriterSink());
          } else {
            sink.reset(new kythe::KindexWriterSink());
          }
          index_writer.WriteIndex(std::move(sink), main_source_file, transcript,
                                  source_files, header_search_info, had_errors);
        });
    clang::tooling::ToolInvocation invocation(final_args, extractor.release(),
                                              file_manager.get());
    if (map_builtin_resources) {
      kythe::MapCompilerResources(&invocation, kBuiltinResourceDirectory);
    }
    invocation.run();
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
