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

#ifndef KYTHE_CXX_COMMON_FILE_VNAME_GENERATOR_H_
#define KYTHE_CXX_COMMON_FILE_VNAME_GENERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "kythe/proto/storage.pb.h"
#include "re2/re2.h"

namespace kythe {

/// \brief Generates file VNames based on user-configurable paths and templates.
class FileVNameGenerator {
 public:
  FileVNameGenerator();

  /// \brief Adds configuration entries from the json-encoded `json_string`.
  /// \param json_string The string containing the configuration to add.
  /// \param error_text Non-null. Will be set to text describing any errors.
  /// \return false if the string could not be parsed.
  bool LoadJsonString(const std::string &json_string, std::string *error_text);

  /// \brief Returns a base VName for a given file path (or an empty VName if
  /// no configuration rule matches the path).
  kythe::proto::VName LookupBaseVName(const std::string &path) const;

  /// \brief Returns a VName for the given file path.
  kythe::proto::VName LookupVName(const std::string &path) const;

 private:
  /// \brief A command to use when building a result string.
  struct StringConsNode {
    enum class Kind {
      kEmitText,        ///< Emit text verbatim.
      kUseSubstitution  ///< Copy from a regex capture.
    } kind;
    /// Text to emit if `kind` is `kEmitText`.
    std::string raw_text;
    /// Substitution to use if `kind` is `kUseSubstitution`.
    int capture_index;
  };
  using StringConsRule = std::vector<StringConsNode>;
  /// \brief Applies a `StringConsRule` using the result of a regex match.
  /// \param rule The rule to apply.
  /// \param argv The `RE2::Arg` results from the regex match.
  /// \param argc The length of the array `argv`.
  std::string ApplyRule(const StringConsRule &rule,
                        const re2::StringPiece *argv, int argc) const;
  /// \brief Parses `rule` into a `StringConsRule`.
  /// \param rule The rule to parse.
  /// \param max_capture_index The maximum utterable capture index.
  /// \param result The StringConsRule to parse into.
  /// \param error_text Set to a descriptive message if parsing fails.
  /// \return false if we could not parse a rule.
  bool ParseRule(const std::string &rule, int max_capture_index,
                 StringConsRule *result, std::string *error_text);
  /// \brief A rule to apply to certain paths.
  struct VNameRule {
    /// The pattern used to match against a path.
    std::shared_ptr<re2::RE2> pattern;
    /// Substitution pattern used to construct the corpus.
    StringConsRule corpus;
    /// Substitution pattern used to construct the root.
    StringConsRule root;
    /// Substitution pattern used to construct the path.
    StringConsRule path;
  };
  /// The rules to apply to incoming paths. The first one to match is used.
  std::vector<VNameRule> rules_;
  /// Used internally to find substitution markers when compiling rules.
  RE2 substitution_matcher_{"([^@]*)@([^@]+)@"};
};
}  // namespace kythe

#endif
