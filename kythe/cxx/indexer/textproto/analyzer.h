/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_TEXTPROTO_ANALYZER_H_
#define KYTHE_CXX_INDEXER_TEXTPROTO_ANALYZER_H_

#include <cstdio>
#include <string>

#include "absl/functional/function_ref.h"
#include "absl/status/status.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/proto/analysis.pb.h"
#include "plugin.h"

namespace kythe {
namespace lang_textproto {

// The canonical name for the textproto language in Kythe.
extern const absl::string_view kLanguageName;

/// Analyzes the textproto file described by @unit and emits graph facts to
/// @recorder.
///
/// The basic indexing flow is as follows:
/// * Build a DescriptorPool from all protos in the compilation unit.
/// * Find the descriptor for the textproto's main message by name.
/// * Construct an empty message instance from the descriptor.
/// * Parse the textproto into our empty message using TextFormat::Parser with
///   locations recorded to a ParseInfoTree. The parser uses the DescriptorPool
///   to lookup field descriptors and extensions.
/// * For each field in the descriptor, see if the ParseInfoTree saw one in the
///   input. If so, add an anchor node and associate it with the proto
///   descriptor with a “ref” edge.
/// * Repeat the above step recursively for any fields that are messages.
///
/// \param unit The compilation unit specifying the textproto and the
/// protos that define its schema.
/// \param file_data The file contents of the textproto and relevant protos.
/// \param The name of the message type that defines the schema for the
/// textproto file (including namespace).
absl::Status AnalyzeCompilationUnit(const proto::CompilationUnit& unit,
                                    const std::vector<proto::FileData>& files,
                                    KytheGraphRecorder* recorder);

// Callback function to instantiate plugins for a given proto message type.
using PluginLoadCallback =
    absl::FunctionRef<std::vector<std::unique_ptr<Plugin>>(
        absl::string_view msg_name, const google::protobuf::Message& proto)>;

// Override for AnalyzeCompilationUnit() that accepts a PluginLoadCallback for
// loading plugins.
absl::Status AnalyzeCompilationUnit(PluginLoadCallback plugin_loader,
                                    const proto::CompilationUnit& unit,
                                    const std::vector<proto::FileData>& files,
                                    KytheGraphRecorder* recorder);

}  // namespace lang_textproto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_TEXTPROTO_ANALYZER_H_
