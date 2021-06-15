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

#ifndef KYTHE_CXX_COMMON_FILE_VNAME_GENERATOR_H_
#define KYTHE_CXX_COMMON_FILE_VNAME_GENERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "kythe/proto/storage.pb.h"
#include "re2/re2.h"

namespace kythe {

/// \brief Generates file VNames based on user-configurable paths and templates.
class FileVNameGenerator {
 public:
  explicit FileVNameGenerator() = default;

  /// \brief Adds configuration entries from the json-encoded `json_string`.
  /// \param json_string The string containing the configuration to add.
  /// \param error_text Non-null. Will be set to text describing any errors.
  /// \return false if the string could not be parsed.
  bool LoadJsonString(absl::string_view data, std::string* error_text);
  absl::Status LoadJsonString(absl::string_view data);

  /// \brief Returns a base VName for a given file path (or an empty VName if
  /// no configuration rule matches the path).
  kythe::proto::VName LookupBaseVName(absl::string_view path) const;

  /// \brief Returns a VName for the given file path.
  kythe::proto::VName LookupVName(absl::string_view path) const;

  /// \brief Sets the default base VName to use when no rules match.
  void set_default_base_vname(const kythe::proto::VName& default_vname) {
    default_vname_ = default_vname;
  }

 private:
  /// \brief A rule to apply to certain paths.
  struct VNameRule {
    /// The pattern used to match against a path.
    std::shared_ptr<re2::RE2> pattern;
    /// Substitution pattern used to construct the corpus.
    std::string corpus;
    /// Substitution pattern used to construct the root.
    std::string root;
    /// Substitution pattern used to construct the path.
    std::string path;
  };
  /// The rules to apply to incoming paths. The first one to match is used.
  std::vector<VNameRule> rules_;
  /// The default base VName to use when no rules match.
  kythe::proto::VName default_vname_;
};
}  // namespace kythe

#endif
