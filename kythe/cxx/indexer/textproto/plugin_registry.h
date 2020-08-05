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

#include <string>

#include "plugin.h"

namespace kythe {
namespace lang_textproto {

// PluginRegistry is a utility class for mapping and instantiating indexer
// plugins based on the top-level proto message they apply to.
class PluginRegistry {
 public:
  PluginRegistry() {}

  // Instantiate a set of plugins for the given proto message type.
  std::vector<std::unique_ptr<Plugin>> ConstructPluginsForMessage(
      const std::string& message_name);

  // Register a plugin to be run on certain types of textprotos.
  template <typename PLUGIN_TYPE>
  void RegisterPlugin(const std::string& name,
                      std::vector<std::string> proto_messages) {
    RegisterPlugin(name, proto_messages,
                   [](const google::protobuf::Message& proto) {
                     return std::make_unique<PLUGIN_TYPE>(proto);
                   });
  }

 private:
  using Instantiator = std::function<std::unique_ptr<Plugin>()>;

  void RegisterPlugin(const std::string& name,
                      std::vector<std::string> proto_messages, Instantiator c);

  struct Entry {
    std::string name;
    std::vector<std::string> proto_messages;
    Instantiator create;
  };

  std::map<std::string, Entry> plugins_;
  // map of proto message name to plugin names
  std::map<std::string, std::vector<std::string>> plugins_by_msg_;
};

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_REGISTRY_H_
