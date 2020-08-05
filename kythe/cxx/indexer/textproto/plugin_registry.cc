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

#include "plugin_registry.h"

#include "glog/logging.h"

namespace kythe {
namespace lang_textproto {

std::vector<std::unique_ptr<Plugin>> PluginRegistry::ConstructPluginsForMessage(
    const std::string& message_name, const google::protobuf::Message& proto) {
  std::vector<std::unique_ptr<Plugin>> plugins;
  for (const auto& plugin_name : plugins_by_msg_.find(message_name)->second) {
    plugins.push_back(plugins_.at(plugin_name).create(proto));
  }
  return plugins;
}

void PluginRegistry::RegisterPlugin(const std::string& name,
                                    std::vector<std::string> proto_messages,
                                    Instantiator c) {
  if (plugins_.find(name) != plugins_.end()) {
    LOG(INFO) << "Attempt to re-register plugin with the same name: '" << name
              << "'. skipping...";
    return;
  }

  plugins_.emplace(name, Entry{name, proto_messages, c});
  for (auto& msg : proto_messages) {
    plugins_by_msg_[msg].push_back(name);
  }
}

}  // namespace lang_textproto
}  // namespace kythe
