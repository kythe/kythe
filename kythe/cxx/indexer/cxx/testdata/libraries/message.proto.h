// This mimicks a generated protobuf:
//
// package some.package;
//
// message Inner {
//   optional int32 my_int = 1;
//   extends proto2.bridge.MessageSet {
//     Outer message_set_extension = 1234321;
//   }
// }
// message Outer {
//   optional Inner inner = 1;
//   optional int my_int = 2;
//   optional proto2.bridge.MessageSet = 3;
// }
//
// extends Inner {
//   float global_extension = 123;
// }
#ifndef KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_
#define KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_

#include "proto_extension.h"

class string;

namespace some {
namespace package {

//- @Inner defines/binding InnerProto
class Inner {
 public:
  //- @my_int defines/binding MyIntAccessor
  //- MyIntAccessor childof InnerProto
  int my_int() const;

  //- @message_set_extension defines/binding MsgSetExtId
  //- MsgSetExtId childof InnerProto
  static ::proto2::internal::ExtensionIdentifier<
      ::proto2::bridge::MessageSet,
      ::proto2::internal::MessageTypeTraits<::some::package::Inner>>
      message_set_extension;
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
  //- @msg_set defines/binding MsgSetAccessor
  //- MsgSetAccessor childof OuterProto
  const proto2::bridge::MessageSet& msg_set() const;
};

//- @message_set_extension defines/binding GlobalExtId
extern ::proto2::internal::ExtensionIdentifier<
    ::some::package::Inner, ::proto2::internal::PrimitiveTypeTraits<float>>
    global_extension;

}  // namespace package
}  // namespace some

#endif  // KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_MESSAGE_PROTO_H_
