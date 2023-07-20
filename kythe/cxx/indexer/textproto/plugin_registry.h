/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_REGISTRY_H_
#define KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_REGISTRY_H_

#include <string_view>

#include "google/protobuf/descriptor.h"
#include "kythe/cxx/indexer/textproto/plugin.h"

namespace kythe {
namespace lang_textproto {

// Simple PluginLoadCallback function that loads plugins relevant to the given
// proto message. New plugins should be added to the implementation of this
// function.
std::vector<std::unique_ptr<Plugin>> LoadRegisteredPlugins(
    const google::protobuf::Message& proto);

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_REGISTRY_H_
