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

#include "kythe/cxx/extractor/cxx_details.h"

#include "absl/log/log.h"

namespace kythe {

/// The type URI for C++ details.
const char kCxxCompilationUnitDetailsURI[] =
    "kythe.io/proto/kythe.proto.CxxCompilationUnitDetails";

void HeaderSearchInfo::CopyTo(
    kythe::proto::CxxCompilationUnitDetails* cxx_details) const {
  auto* info = cxx_details->mutable_header_search_info();
  info->set_first_angled_dir(angled_dir_idx);
  info->set_first_system_dir(system_dir_idx);
  for (const auto& path : paths) {
    auto* dir = info->add_dir();
    dir->set_path(path.path);
    dir->set_characteristic_kind(path.characteristic_kind);
    dir->set_is_framework(path.is_framework);
  }
  for (const auto& prefix : system_prefixes) {
    auto* proto = cxx_details->add_system_header_prefix();
    proto->set_is_system_header(prefix.second);
    proto->set_prefix(prefix.first);
  }
}

bool HeaderSearchInfo::CopyFrom(
    const kythe::proto::CxxCompilationUnitDetails& cxx_details) {
  paths.clear();
  system_prefixes.clear();
  const auto& info = cxx_details.header_search_info();
  angled_dir_idx = info.first_angled_dir();
  system_dir_idx = info.first_system_dir();
  for (const auto& dir : info.dir()) {
    paths.emplace_back(
        HeaderSearchInfo::Path{dir.path(),
                               static_cast<clang::SrcMgr::CharacteristicKind>(
                                   dir.characteristic_kind()),
                               dir.is_framework()});
  }
  for (const auto& prefix : cxx_details.system_header_prefix()) {
    system_prefixes.emplace_back(prefix.prefix(), prefix.is_system_header());
  }
  return (angled_dir_idx <= system_dir_idx && system_dir_idx <= paths.size());
}

bool HeaderSearchInfo::CopyFrom(
    const clang::HeaderSearchOptions& header_search_options,
    const clang::HeaderSearch& header_search_info) {
  paths.clear();
  system_prefixes.clear();
  angled_dir_idx = header_search_info.search_dir_size();
  system_dir_idx = header_search_info.search_dir_size();
  unsigned cur_dir_idx = 0;
  // Clang never sets no_cur_dir_search to true? (see InitHeaderSearch.cpp)
  bool no_cur_dir_search = false;
  auto first_angled_dir = header_search_info.angled_dir_begin();
  auto first_system_dir = header_search_info.system_dir_begin();
  auto last_dir = header_search_info.system_dir_end();
  for (const auto& prefix : header_search_options.SystemHeaderPrefixes) {
    system_prefixes.emplace_back(prefix.Prefix, prefix.IsSystemHeader);
  }
  for (auto iter = header_search_info.search_dir_begin(); iter != last_dir;
       ++cur_dir_idx, ++iter) {
    if (iter == first_angled_dir) {
      angled_dir_idx = cur_dir_idx;
    } else if (iter == first_system_dir) {
      system_dir_idx = cur_dir_idx;
    }
    switch (iter->getLookupType()) {
      case clang::DirectoryLookup::LT_NormalDir:
        paths.push_back(HeaderSearchInfo::Path{
            std::string(iter->getName()), iter->getDirCharacteristic(), false});
        break;
      case clang::DirectoryLookup::LT_Framework:
        paths.push_back(HeaderSearchInfo::Path{
            std::string(iter->getName()), iter->getDirCharacteristic(), true});
        break;
      default:  // clang::DirectoryLookup::LT_HeaderMap:
        // TODO(zarko): Support LT_HeaderMap.
        LOG(WARNING) << "Can't reproduce include lookup state: "
                     << iter->getName().str() << " is not a normal directory.";
        return false;
    }
  }
  return true;
}

}  // namespace kythe
