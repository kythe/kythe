#include "kythe/testdata/indexers/proto/testdata3.pb.h"

//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata3.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3.pb.h", "")
//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata3a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3a.pb.h", "")
//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata3b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3b.pb.h", "")

//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3a.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3b.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3a.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata3.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata3b.pb.h", "") }
void fn() {
    using namespace ::pkg::proto3;

    //- @Container ref CxxContainerMessage
    Container msg;

    //- @contained ref CxxGetContainedField
    msg.contained();

    //- @contained2 ref CxxGetContained2Field
    msg.contained2();
}
//- ContainerMessageField generates CxxGetContainedField
//- Container2MessageField generates CxxGetContained2Field
//- ContainerMessage generates CxxContainerMessage
