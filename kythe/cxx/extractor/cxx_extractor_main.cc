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

// cxx_extractor is meant to be a drop-in replacement for clang/gcc's frontend.
// It collects all of the resources that clang would use to compile a single
// source file (as determined by the command line arguments) and produces a
// .kzip file.
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
#include "gflags/gflags.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/extractor/language.h"

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  gflags::SetVersionString("0.1");
  std::vector<std::string> args(argv, argv + argc);
  kythe::ExtractorConfiguration config;
  config.SetArgs(args);
  config.InitializeFromEnvironment();
  bool success = config.Extract(kythe::supported_language::Language::kCpp);
  google::protobuf::ShutdownProtobufLibrary();
  return success ? 0 : 1;
}
