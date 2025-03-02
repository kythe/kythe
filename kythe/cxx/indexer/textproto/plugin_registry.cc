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

#include "absl/flags/flag.h"
#include "kythe/cxx/indexer/textproto/plugins/example/plugin.h"

ABSL_FLAG(bool, enable_example_plugin, false,
          "Enable the 'example' textproto plugin");

namespace kythe {
namespace lang_textproto {

std::vector<std::unique_ptr<Plugin>> LoadRegisteredPlugins(
    const google::protobuf::Message& proto) {
  std::vector<std::unique_ptr<Plugin>> plugins;
  absl::string_view msg_name = proto.GetDescriptor()->full_name();
  if (absl::GetFlag(FLAGS_enable_example_plugin) &&
      msg_name == "kythe_plugin_example.Person") {
    plugins.push_back(
        std::make_unique<kythe::lang_textproto::ExamplePlugin>(proto));
  }
  return plugins;
}

}  // namespace lang_textproto
}  // namespace kythe
