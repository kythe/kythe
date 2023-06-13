#include "kythe/testdata/indexers/proto/testdata2.proto.h"

//- vname("",_, "", "kythe/testdata/indexers/proto/testdata2.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata2.proto.h", "")
void fn() {
    using namespace ::pkg::proto2;

    //- @OuterMessage ref CxxOuterMessage
    OuterMessage msg;
    //- @ExtensionMessage ref CxxExtensionMessage
    //- @extension ref CxxExtensionField
    msg.HasExtension(ExtensionMessage::extension);
}
//- OuterMessage generates CxxOuterMessage
//- ExtensionMessage generates CxxExtensionMessage
//- ExtensionField generates CxxExtensionField
