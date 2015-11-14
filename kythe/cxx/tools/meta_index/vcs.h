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

#ifndef KYTHE_CXX_TOOLS_META_INDEX_VCS_H_
#define KYTHE_CXX_TOOLS_META_INDEX_VCS_H_

#include <map>
#include <string>

#include "rapidjson/document.h"

namespace kythe {

/// \brief Provides read-only access to various version control ephemera.
class VcsClient {
 public:
  /// \param cache_path Local directory to store cached queries.
  explicit VcsClient(const std::string &cache_path);

  /// \brief Get a mapping from tag name to revision ID.
  /// \param vcs_uri The version control system URI to use
  /// \param tags The map to fill
  /// \return true on success.
  bool GetTags(const std::string &vcs_uri,
               std::map<std::string, std::string> *tags);

  /// \brief Get the revision ID from a local path.
  /// \param path The local path to a file controlled by a VCS.
  /// \return a nonempty string on success.
  std::string GetRevisionID(const std::string &path);

  /// \brief Sets whether VcsClient may touch the network.
  void set_allow_network(bool allowed) { allow_network_ = allowed; }

 private:
  /// Returns the cached object for some URI.
  bool GetCacheForUri(const std::string &vcs_uri, rapidjson::Value *val);
  /// Updates the cached object for some URI.
  void UpdateCacheForUri(const std::string &vcs_uri, rapidjson::Value &val);
  /// Where to store the cache (if non-empty).
  std::string cache_path_;
  /// The state of the cache.
  rapidjson::Document cache_;
  /// Whether we're allowed to touch the network.
  bool allow_network_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_TOOLS_META_INDEX_VCS_H_
