// Checks that the content of proto string literals is indexed.
#include "message.proto.h"
#include "parsetextproto.h"

class string;

void f() {
  const some::package::Outer msg =
      proto2::contrib::parse_proto::ParseTextProtoOrDie(
          //- @inner ref InnerAccessor
          " inner {"
          //- @my_int ref MyIntAccessor
          "  my_int: 3\n"
          " }"
          //- @my_string ref MyStringAccessor
          " my_string: 'blah'"
          //- @msg_set ref MsgSetAccessor
          " msg_set { "
          //- @"some.package.Inner" ref MsgSetExtId
          "  [some.package.Inner] {"
          //- @my_int ref MyIntAccessor
          "    my_int: 6\n"
          //- @"some.package.global_extension" ref GlobalExtId
          "    [some.package.global_extension]: 0.9\n"
          "  }"
          " }"
          //- @"some.package.Inner" ref InnerProto
          " [type.googleapis.com/some.package.Inner] {"
          //- @my_int ref MyIntAccessor
          "  my_int: 9\n"
          " }");
  //- @my_string ref MyStringAccessor
  msg.my_string();
  //- @inner ref InnerAccessor
  const auto& minn = msg.inner();
  //- @my_int ref MyIntAccessor
  minn.my_int();
}

void g() {
  const some::package::Outer msg = PARSE_TEXT_PROTO(
      //- @inner ref InnerAccessor
      " inner {"
      //- @my_int ref MyIntAccessor
      "  my_int: 3\n"
      " }"
      //- @my_string ref MyStringAccessor
      " my_string: 'blah'");
  //- @my_string ref MyStringAccessor
  msg.my_string();
  //- @inner ref InnerAccessor
  const auto& minn = msg.inner();
  //- @my_int ref MyIntAccessor
  minn.my_int();
}
