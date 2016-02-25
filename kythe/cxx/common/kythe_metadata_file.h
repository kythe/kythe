/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_KYTHE_METADATA_FILE_H_
#define KYTHE_CXX_COMMON_KYTHE_METADATA_FILE_H_

#include "kythe/proto/storage.pb.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "rapidjson/document.h"

#include <map>
#include <memory>

namespace kythe {

class MetadataFile {
 public:
  /// Loads a .meta file from its JSON representation.
  /// \return null on failure.
  static std::unique_ptr<MetadataFile> LoadFromJSON(llvm::StringRef json,
                                                    std::string *error_text);

  /// \brief A single metadata rule.
  struct Rule {
    unsigned begin;        ///< Beginning of the range to match.
    unsigned end;          ///< End of the range to match.
    std::string edge_in;   ///< Edge kind to match from anchor over [begin,end).
    std::string edge_out;  ///< Edge to create.
    proto::VName vname;    ///< VName to create edge to or from.
    bool reverse_edge;     ///< If false, draw edge to vname; if true, draw
                           ///< from.
  };

  /// Rules to apply keyed on `begin`.
  const std::multimap<unsigned, Rule> &rules() const { return rules_; }

 private:
  bool LoadMetaElement(const rapidjson::Value &value, std::string *error_text);

  /// Rules to apply keyed on `begin`.
  std::multimap<unsigned, Rule> rules_;
};

/// \brief Converts from arbitrary metadata formats to those supported by Kythe.
///
/// Some metadata producers may be unable to directly generate Kythe metadata.
/// It may also be difficult to ensure that the data they do produce is
/// converted before a Kythe tool runs. This interface allows tools to
/// support arbitrary metadata formats by converting them to kythe::MetadataFile
/// instances on demand.
class MetadataSupport {
 public:
  virtual ~MetadataSupport() {}
  /// \brief Attempt to parse the file named `filename` with contents in
  /// `buffer`.
  /// \return A `MetadataFile` on success; otherwise, null.
  virtual std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string &filename, const llvm::MemoryBuffer *buffer) {
    return nullptr;
  }
};

/// \brief A collection of metadata support implementations.
///
/// Each `MetadataSupport` is tried in vector order. The first one to return
/// a non-null result from `ParseFile` is elected to provide metadata for a
/// given (`filename`, `buffer`) pair.
using MetadataSupports = std::vector<std::unique_ptr<MetadataSupport>>;

/// \brief Enables support for raw JSON-encoded metadata files.
class KytheMetadataSupport : public MetadataSupport {
 public:
  std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string &filename, const llvm::MemoryBuffer *buffer) override;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KYTHE_METADATA_FILE_H_
