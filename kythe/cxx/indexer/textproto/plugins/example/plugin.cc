#include "plugin.h"

namespace kythe {
namespace lang_textproto {

absl::Status ExamplePlugin::AnalyzeStringField(
    PluginApi* api, const proto::VName& file_vname,
    const google::protobuf::FieldDescriptor& field, re2::StringPiece input) {
  // Create an anchor covering the field value's text span.
  proto::VName anchor_vname = api->CreateAndAddAnchorNode(file_vname, input);

  auto target_vname = api->VNameForDescriptor(&field);
  if (!target_vname.ok()) return target_vname.status();

  // Add a ref edge from the anchor to the proto field descriptor.
  api->recorder()->AddEdge(VNameRef(anchor_vname), EdgeKindID::kRef,
                           VNameRef(*target_vname));

  return absl::OkStatus();
}

}  // namespace lang_textproto
}  // namespace kythe
