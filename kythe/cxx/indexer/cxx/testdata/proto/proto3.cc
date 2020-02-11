#include "kythe/testdata/indexers/proto/testdata3.pb.h"

//- vname("",_, "", "kythe/testdata/indexers/proto/testdata3.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3.pb.h", "")
//- vname("",_, "", "kythe/testdata/indexers/proto/testdata3a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3a.pb.h", "")
void fn() {
    using namespace ::pkg::proto3;

    //- @Container ref CxxContainerMessage
    Container msg;

    //- @contained ref CxxGetContainedField
    msg.contained();
}
//- ContainerMessageField generates CxxGetContainedField
//- ContainerMessage generates CxxContainerMessage

// required since mentioned in testdata3a.pb.h, transitively included
//- ThingMessage.node/kind record
