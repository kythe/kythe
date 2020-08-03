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

#ifndef KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_H_
#define KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_H_

#include <cstdio>
#include <string>

#include "absl/status/status.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/status_or.h"
#include "kythe/cxx/indexer/proto/vname_util.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {
namespace lang_textproto {

// The plugin's interface to the indexer.
class PluginApi {
 public:
  virtual proto::VName CreateAndAddAnchorNode(const proto::VName& file,
                                              int begin, int end) = 0;

  virtual proto::VName CreateAndAddAnchorNode(const proto::VName& file_vname,
                                              absl::string_view sp) = 0;

  virtual KytheGraphRecorder* recorder() = 0;

  virtual void EmitDiagnostic(const proto::VName& file_vname,
                              absl::string_view signature,
                              absl::string_view msg) = 0;

  virtual absl::optional<proto::VName> VNameForRelPath(
      absl::string_view simplified_path) const = 0;

  // Create a VName for a proto descriptor.
  template <typename SomeDescriptor>
  StatusOr<proto::VName> VNameForDescriptor(const SomeDescriptor* descriptor) {
    absl::Status vname_lookup_status = absl::OkStatus();
    proto::VName vname = ::kythe::lang_proto::VNameForDescriptor(
        descriptor, [this, &vname_lookup_status](const std::string& path) {
          auto v = VNameForRelPath(path);
          if (!v.has_value()) {
            vname_lookup_status = absl::UnknownError(
                absl::StrCat("Unable to lookup vname for rel path: ", path));
            return proto::VName();
          }
          return *v;
        });
    return vname_lookup_status.ok() ? StatusOr<proto::VName>(vname)
                                    : vname_lookup_status;
  }
};

// All plugins must subclass this class.
class Plugin {
 public:
  virtual ~Plugin() {}
  // Main entrypoint for plugins.
  virtual absl::Status AnalyzeStringField(
      PluginApi* api, const proto::VName& file_vname,
      const google::protobuf::FieldDescriptor& field,
      absl::string_view input) = 0;
};

// Callback function to instantiate a plugin by name. Returns nil if the name
// doesn't refer to a valid plugin.
typedef std::function<std::unique_ptr<Plugin>(absl::string_view name)>
    PluginLoadCallback;

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_PLUGIN_H_
