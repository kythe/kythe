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

#ifndef KYTHE_CXX_INDEXER_TEXTPROTO_PLUGINS_EXAMPLE_PLUGIN_H_
#define KYTHE_CXX_INDEXER_TEXTPROTO_PLUGINS_EXAMPLE_PLUGIN_H_

#include "kythe/cxx/indexer/textproto/plugin.h"

namespace kythe {
namespace lang_textproto {

// The example plugin demonstrates the implementation of a textproto plugin and
// serves as a test that the plugin system works.
//
// The proto schema defines a very simple social graph. The single message
// `Person` has a `name` field and a repeated `friend`s field. The plugin
// defines a new kythe semantic node for each person, using their name as the
// "signature". Each 'friend' entry in the textproto is treated as a `ref` to
// the semantic node representing the person of that name. Note that these
// semantic nodes have language="textproto_plugin_example" rather than
// "textproto".
class ExamplePlugin : public Plugin {
 public:
  ExamplePlugin(const google::protobuf::Message& proto) {}

  absl::Status AnalyzeStringField(
      PluginApi* api, const proto::VName& file_vname,
      const google::protobuf::FieldDescriptor& field,
      const std::vector<StringToken>& tokens) override;
};

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_PLUGINS_EXAMPLE_PLUGIN_H_
