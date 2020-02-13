#include "kythe/testdata/indexers/proto/testdata4a.pb.h"
#include "kythe/testdata/indexers/proto/testdata4b.pb.h"

//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata4a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4a.pb.h", "")
//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata4b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4b.pb.h", "")
//-    vname("",_, "", "kythe/testdata/indexers/proto/testdata4c.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4c.pb.h", "")
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4b.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4a.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4c.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4a.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4b.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4c.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4c.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4a.pb.h", "") }
//- !{ vname("",_, "", "kythe/testdata/indexers/proto/testdata4c.proto","") generates vname("", _, "bazel-out/bin", "kythe/testdata/indexers/proto/testdata4b.pb.h", "") }
void fn() {
  using namespace ::pkg::proto4a;

  //- @Thing1 ref Thing1Message?
  //- Thing1Message.node/kind record
  Thing1 thing1;

  using namespace ::pkg::proto4b;

  //- @Thing2 ref Thing2Message?
  //- Thing2Message.node/kind record
  Thing2 thing2;
}
