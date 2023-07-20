/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include <map>
#include <memory>
#include <optional>

#include "absl/strings/string_view.h"
#include "kythe/proto/metadata.pb.h"
#include "kythe/proto/storage.pb.h"
#include "rapidjson/document.h"

namespace kythe {

class MetadataFile {
 public:
  /// \brief An additional semantic to apply to the given (C++) node.
  enum class Semantic {
    kNone,       ///< No special semantics.
    kWrite,      ///< Write semantics.
    kReadWrite,  ///< Read+write semantics.
    kTakeAlias,  ///< Alias-taking semantics.
  };

  /// \brief A single metadata rule.
  struct Rule {
    unsigned begin = 0;    ///< Beginning of the range to match.
    unsigned end = 0;      ///< End of the range to match.
    std::string edge_in;   ///< Edge kind to match from anchor over [begin,end).
    std::string edge_out;  ///< Edge to create.
    proto::VName vname;    ///< VName to create edge to or from.
    bool reverse_edge = false;  ///< If false, draw edge to vname; if true, draw
                                ///< from.
    bool generate_anchor = false;  ///< If this rule should generate an anchor.
    unsigned anchor_begin = 0;     ///< The beginning of the anchor.
    unsigned anchor_end = 0;       ///< The end of the anchor.
    bool whole_file = false;       ///< Whether to ignore begin/end
    Semantic semantic =
        Semantic::kNone;  ///< Whether to apply special semantics.
  };

  /// Creates a new MetadataFile from a list of rules ranging from `begin` to
  /// `end`.
  template <typename InputIterator>
  static std::unique_ptr<MetadataFile> LoadFromRules(absl::string_view id,
                                                     InputIterator begin,
                                                     InputIterator end) {
    std::unique_ptr<MetadataFile> meta_file = std::make_unique<MetadataFile>();
    meta_file->id_ = std::string(id);
    for (auto rule = begin; rule != end; ++rule) {
      if (rule->whole_file) {
        meta_file->file_scope_rules_.push_back(*rule);
      } else {
        meta_file->rules_.emplace(rule->begin, *rule);
      }
    }
    return meta_file;
  }

  //// Attempts to convert `mapping` to a `Rule`.
  static std::optional<MetadataFile::Rule> LoadMetaElement(
      const kythe::proto::metadata::MappingRule& mapping);

  /// Rules to apply keyed on `begin`.
  const std::multimap<unsigned, Rule>& rules() const { return rules_; }

  /// File-scoped rules.
  const std::vector<Rule>& file_scope_rules() const {
    return file_scope_rules_;
  }

  absl::string_view id() const { return id_; }

 private:
  /// Rules to apply keyed on `begin`.
  std::multimap<unsigned, Rule> rules_;

  std::vector<Rule> file_scope_rules_;

  std::string id_;
};

/// \brief Provides interested MetadataSupport classes with the ability to
/// look up VNames for arbitrary required inputs (including metadata files).
/// \return true and merges `path`'s VName into `out` on success; false on
/// failure.
using VNameLookup =
    std::function<bool(const std::string& path, proto::VName* out)>;

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
  /// \brief Attempt to parse the file originally named `raw_filename` with
  /// decoded filename `filename` and contents in `buffer` to be applied
  /// to a target file with contents `target_buffer`.
  /// \return A `MetadataFile` on success; otherwise, null.
  virtual std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string& raw_filename, const std::string& filename,
      absl::string_view buffer, absl::string_view target_buffer) {
    return nullptr;
  }

  /// \brief Use `lookup` when generating VNames (if necessary).
  virtual void UseVNameLookup(VNameLookup lookup) {}
};

/// \brief A collection of metadata support implementations.
///
/// Each `MetadataSupport` is tried in order. The first one to return
/// a non-null result from `ParseFile` is elected to provide metadata for a
/// given (`filename`, `buffer`) pair.
///
/// If the metadata file ends in .h, we assume that it is a valid C++ header
/// that begins with a comment marker followed immediately by a base64-encoded
/// buffer. We will decode and parse this buffer using the filename with the .h
/// removed (as some support implementations discriminate based on extension).
/// The comment mark, the contents of the comment, and any relevant control
/// characters must be 7-bit ASCII. The character set used for base64 encoding
/// is A-Za-z0-9+/ in that order, possibly followed by = or == for padding.
///
/// If search_string is set, `buffer` will be scanned for a comment marker
/// followed by a space followed by search_string. Should this comment be
/// found, it will be decoded as above.
///
/// If the comment is a //-style comment, the base64 string must be unbroken.
/// If the comment is a /* */-style comment, newlines (\n) are permitted.
class MetadataSupports {
 public:
  void Add(std::unique_ptr<MetadataSupport> support) {
    supports_.push_back(std::move(support));
  }

  std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string& filename, absl::string_view buffer,
      const std::string& search_string, absl::string_view target_buffer) const;

  void UseVNameLookup(VNameLookup lookup) const;

 private:
  std::vector<std::unique_ptr<MetadataSupport>> supports_;
};

/// \brief Enables support for raw JSON-encoded metadata files.
class KytheMetadataSupport : public MetadataSupport {
 public:
  std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string& raw_filename, const std::string& filename,
      absl::string_view buffer, absl::string_view target_buffer) override;

 private:
  /// \brief Load the JSON-encoded metadata from `json`.
  /// \return null on failure.
  static std::unique_ptr<MetadataFile> LoadFromJSON(absl::string_view id,
                                                    absl::string_view json);
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KYTHE_METADATA_FILE_H_
