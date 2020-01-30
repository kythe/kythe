#include "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/testdata2.pb.h"

//- vname("",_, "", "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/testdata2.proto","") generates vname("", _, "bazel-out/bin", "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/testdata2.pb.h", "")
void fn() {
    using namespace ::pkg::proto2;

    //- @OuterMessage ref CxxOuterMessage?
    OuterMessage msg;
    //- @ExtensionMessage ref CxxExtensionMessage?
    //- @extension ref CxxExtensionField?
    msg.HasExtension(ExtensionMessage::extension);
}
//- OuterMessage? generates CxxOuterMessage?
//- ExtensionMessage? generates CxxExtensionMessage?
//- ExtensionField? generates CxxExtensionField?
