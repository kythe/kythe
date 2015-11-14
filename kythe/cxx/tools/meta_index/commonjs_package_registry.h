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

#ifndef KYTHE_CXX_TOOLS_META_INDEX_COMMONJS_PACKAGE_REGISTRY_H_
#define KYTHE_CXX_TOOLS_META_INDEX_COMMONJS_PACKAGE_REGISTRY_H_

#include "kythe/cxx/common/net_client.h"

namespace kythe {

/// \brief A client for a CommonJS package registry.
///
/// See <http://wiki.commonjs.org/wiki/Packages/Registry>.
class CommonJsPackageRegistryClient : public JsonClient {
 public:
  /// \param registry_uri The registry to use (e.g.,
  /// https://skimdb.npmjs.com/registry, which is a replica of
  /// http://registry.npmjs.org/).
  explicit CommonJsPackageRegistryClient(const std::string &registry_uri);

  /// \brief Gets all metadata available for some package.
  /// \param package_name The name of the package to look up.
  /// \param document_out The document to fill out on success.
  /// \return true if we got a good response.
  virtual bool GetPackage(const std::string &package_name,
                          rapidjson::Document *document_out);

  /// \brief Set to true if we may touch the network.
  void set_allow_network(bool allowed) { allow_network_ = allowed; }

 private:
  /// The registry we're using.
  std::string registry_uri_;
  /// Whether we're actually allowed to go out to the network.
  bool allow_network_ = false;
};

/// \brief A client that checks a local filesystem cache before trying the
/// network.
class CommonJsPackageRegistryCachingClient
    : public CommonJsPackageRegistryClient {
 public:
  /// \param registry_uri The registry to use.
  /// \param cache_dir The local cache directory to use.
  CommonJsPackageRegistryCachingClient(const std::string &registry_uri,
                                       const std::string &cache_dir);

  bool GetPackage(const std::string &package_name,
                  rapidjson::Document *document_out) override;

 private:
  /// The local cache directory we're using.
  std::string cache_dir_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_TOOLS_META_INDEX_COMMONJS_PACKAGE_REGISTRY_H_
