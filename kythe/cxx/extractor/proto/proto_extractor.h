/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_EXTRACTOR_PROTO_PROTO_EXTRACTOR_H_
#define KYTHE_CXX_EXTRACTOR_PROTO_PROTO_EXTRACTOR_H_

#include "absl/strings/string_view.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/index_writer.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace lang_proto {

class ProtoExtractor {
 public:
  /// Reads KYTHE_VNAMES, KYTHE_CORPUS, and KYTHE_ROOT_DIRECTORY environment
  /// variables and configures vname_gen, corpus, and root_directory
  /// appropriately.
  void ConfigureFromEnv();

  /// \brief Extracts the given list of proto files and all of their imported
  /// dependencies.
  ///
  /// \param proto_filenames A list of toplevel protos to extract. These are
  /// added to the compilation unit's "source_file" and "required_input" lists.
  /// \param index_writer A writer to save all required input files to.
  /// \return A compilation unit with all proto files and --proto_path arguments
  /// recorded. Note that the returned compilation unit is not written to the
  /// index_writer.
  proto::CompilationUnit ExtractProtos(
      const std::vector<std::string>& proto_filenames,
      IndexWriter* index_writer) const;

  /// Search paths where the proto compiler will look for proto files. See
  /// indexer/proto/search_path.h for details.
  std::vector<std::pair<std::string, std::string>> path_substitutions;
  /// Used to generate vnames for each proto file.
  FileVNameGenerator vname_gen;
  /// All paths recorded in the compilation unit will be made relative to this
  /// directory.
  std::string root_directory = ".";
  /// Any path that is not assigned a corpus by the FileVNameGenerator is given
  /// this corpus id.
  std::string corpus;
};

}  // namespace lang_proto
}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_PROTO_PROTO_EXTRACTOR_H_
