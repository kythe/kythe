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

#include "commonjs_package_registry.h"

#include "glog/logging.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "llvm/Support/Path.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include "utils.h"

namespace kythe {

namespace {
/// \brief Escapes \ and / (and =) such that `to_escape` can be used as a path
/// node.
std::string PathEscape(const std::string &to_escape) {
  std::string result;
  result.reserve(to_escape.size());
  for (const auto &ch : to_escape) {
    switch (ch) {
      case '=':
        result.append("==");
        break;
      case '\\':
        result.append("=b");
        break;
      case '/':
        result.append("=s");
        break;
      default:
        result.push_back(ch);
    }
  }
  return result;
}
}

CommonJsPackageRegistryClient::CommonJsPackageRegistryClient(
    const std::string &uri)
    : registry_uri_(uri) {}

bool CommonJsPackageRegistryClient::GetPackage(
    const std::string &package_name, rapidjson::Document *document_out) {
  if (!allow_network_) {
    LOG(WARNING) << "GetPackage request denied access to the network.";
    return false;
  }
  return Request(
      registry_uri_ + "/" + UriEscape(UriEscapeMode::kEscapeAll, package_name),
      false, "", document_out);
}

CommonJsPackageRegistryCachingClient::CommonJsPackageRegistryCachingClient(
    const std::string &uri, const std::string &cache)
    : CommonJsPackageRegistryClient(uri), cache_dir_(cache) {}

bool CommonJsPackageRegistryCachingClient::GetPackage(
    const std::string &package_name, rapidjson::Document *document_out) {
  llvm::SmallString<1024> path_record;
  llvm::sys::path::append(path_record, cache_dir_, PathEscape(package_name));
  if (auto in_file = OpenFile(path_record.str().str().c_str(), "r")) {
    VLOG(0) << "cache hit for " << path_record.str().str();
    char buffer[1024];
    rapidjson::FileReadStream stream(in_file.get(), buffer, sizeof(buffer));
    document_out->ParseStream(stream);
    return true;
  }

  bool found_package =
      CommonJsPackageRegistryClient::GetPackage(package_name, document_out);
  if (found_package) {
    if (auto out_file = OpenFile(path_record.str().str().c_str(), "w")) {
      char buffer[1024];
      {
        rapidjson::FileWriteStream stream(out_file.get(), buffer,
                                          sizeof(buffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);
        document_out->Accept(writer);
      }
    } else {
      LOG(WARNING) << "can't write to cache for " << path_record.str().str();
    }
  }
  return found_package;
}

}  // namespace kythe
