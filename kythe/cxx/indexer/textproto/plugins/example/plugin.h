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
// serves as a test that the plugin system works. It adds an anchor for every
// string field *value*  and a `ref` edge from that to the proto descriptor for
// the field. See `plugin_test.pbtxt` for an example.
class ExamplePlugin : public Plugin {
 public:
  absl::Status AnalyzeStringField(
      PluginApi* api, const proto::VName& file_vname,
      const google::protobuf::FieldDescriptor& field,
      absl::string_view input) override;
};

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_PLUGINS_EXAMPLE_PLUGIN_H_
