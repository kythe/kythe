// This mimicks a generated protobuf:
// message Inner {
//   optional int32 my_int = 1;
// }
// message Outer {
//   optional Inner inner = 1;
//   optional int my_int = 2;
// }
#ifndef KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_
#define KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_

class string;

namespace some {
namespace package {

//- @Inner defines/binding InnerProto
class Inner {
 public:
  //- @my_int defines/binding MyIntAccessor
  //- MyIntAccessor childof InnerProto
  int my_int() const;
};

//- @Outer defines/binding OuterProto
class Outer {
 public:
  //- @inner defines/binding InnerAccessor
  //- InnerAccessor childof OuterProto
  const Inner& inner() const;
  //- @my_string defines/binding MyStringAccessor
  //- MyStringAccessor childof OuterProto
  const string& my_string() const;
};
}  // namespace package
}  // namespace some

#endif  // KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_

