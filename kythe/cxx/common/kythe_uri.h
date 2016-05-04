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

#ifndef KYTHE_CXX_COMMON_KYTHE_URI_H_
#define KYTHE_CXX_COMMON_KYTHE_URI_H_

#include <string>

#include "kythe/cxx/common/vname_ordering.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {

/// \brief Determines the behavior of URI escaping.
enum class UriEscapeMode {
  kEscapeAll,   ///< Escape all reserved characters.
  kEscapePaths  ///< Escape all reserved characters except '/'.
};

/// \brief URI-escapes a string.
/// \param mode The escaping mode to use.
/// \param string The string to escape.
std::string UriEscape(UriEscapeMode mode, const std::string &uri);

/// \brief A Kythe URI.
///
/// URIs are not in 1:1 correspondence with VNames--particularly because
/// the `corpus` component is considered to be a path that can be lexically
/// canonicalized. For example, the corpus "a/b/../c" is equivalent to the
/// corpus "a/c".
class URI {
 public:
  URI() {}
  /// \brief Builds a URI from a `VName`, canonicalizing its `corpus`.
  explicit URI(const kythe::proto::VName &from_vname);

  bool operator==(const URI &o) const { return VNameEquals(vname_, o.vname_); }
  bool operator!=(const URI &o) const { return !VNameEquals(vname_, o.vname_); }

  /// \brief Constructs a URI from an encoded string.
  /// \param uri The string to construct from.
  /// \return (true, URI) on success; (false, empty URI) on failure.
  static std::pair<bool, URI> FromString(const std::string &uri) {
    URI result;
    bool is_ok = result.ParseString(uri);
    return std::make_pair(is_ok, result);
  }

  /// \brief Encodes this URI as a string.
  ///
  /// \return This URI, appropriately escaped.
  std::string ToString() const;

  /// \return This URI as a VName.
  const kythe::proto::VName &v_name() const { return vname_; }

 private:
  /// \brief Attempts to overwrite vname_ using the provided URI string.
  /// \param uri The URI to parse.
  /// \return true on success
  bool ParseString(const std::string &uri);

  /// The VName this URI represents.
  kythe::proto::VName vname_;
};
}  // namespace kythe

#endif
