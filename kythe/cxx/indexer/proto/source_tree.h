/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_PROTO_SOURCE_TREE_H_
#define KYTHE_CXX_INDEXER_PROTO_SOURCE_TREE_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace kythe {

// An implementation of SourceTree that
//  - provides only pre-loaded files and
//  - understands the path substitutions (generalizing search paths) used by
//    the Proto compiler (e.g., for import statements).
// Needed because Proto files will import "foo/bar.proto" and not want to care
// whether the path is actually "bazel-out/long/path/foo/genprotos/bar.proto".
// A ProtoFileParser passes exactly the argument of import statements, leaving
// details like this to some other party (in this case the FileReader).
//
// Takes a sequence of substitutions (s, t) such that paths beginning with s
// (and where the end of s is the end of a path component) can be treated as
// though they instead began with t for purposes of file lookup. This is the
// same as what protoc gets from "-I" and "--proto_path arguments".
// (Often either s is empty, in which case t is essentially a search path, or
// s and t completely specify virtual and actual filenames.)
//
// Substitutions are attempted one at a time (not composed) in the order they
// appear, which should match the order they were specified on the protoc
// command line. If no applicable substitution finds a valid file, the
// filename is considered one more time with no substitution before failing.
class PreloadedProtoFileTree : public google::protobuf::compiler::SourceTree {
 public:
  PreloadedProtoFileTree(
      const std::vector<std::pair<std::string, std::string>>* substitutions,
      absl::flat_hash_map<std::string, std::string>* file_mapping_cache)
      : substitutions_(substitutions),
        file_mapping_cache_(file_mapping_cache) {}

  // disallow copy and assign
  PreloadedProtoFileTree(const PreloadedProtoFileTree&) = delete;
  void operator=(const PreloadedProtoFileTree&) = delete;

  // Add a file's full name (i.e., what any substitutions will map the name(s)
  // by which it is included onto) to the FileReader.
  // Returns false if `filename` was already added.
  bool AddFile(const std::string& filename, const std::string& contents);

  // Load the full contents of `filename` into `contents`, if possible, and
  // return whether this was successful.
  // Note that ProtoFileParser passes the literal argument to import statements
  // as `filename`, leaving FileReaders to do any elaboration (e.g., in this
  // case, applying path substitutions).
  google::protobuf::io::ZeroCopyInputStream* Open(
      absl::string_view filename) override;

  // If Open() returns nullptr, calling this method immediately will return a
  // description of the error.
  std::string GetLastErrorMessage() override { return last_error_; }

  // A wrapper around Open(), that reads the proto file contents into a buffer.
  bool Read(absl::string_view file_path, std::string* out);

 private:
  // All path prefix substitutions to consider.
  const std::vector<std::pair<std::string, std::string>>* const substitutions_;

  // A map of pre-substitution to post-substitution names for all files that
  // have been successfully read via this reader.
  absl::flat_hash_map<std::string, std::string>* file_mapping_cache_;

  // Path (post-substitution) -> file contents.
  absl::flat_hash_map<std::string, std::string> file_map_;

  // A description of the error from the last call to Open() (if any).
  std::string last_error_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_SOURCE_TREE_H_
