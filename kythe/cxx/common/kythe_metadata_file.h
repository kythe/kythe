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
}

#endif  // KYTHE_CXX_COMMON_KYTHE_METADATA_FILE_H_
