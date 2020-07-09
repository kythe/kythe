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
          " my_string: 'blah'");
  //- @my_string ref MyStringAccessor
  msg.my_string();
  //- @inner ref InnerAccessor
  const auto& minn = msg.inner();
  //- @my_int ref MyIntAccessor
  minn.my_int();
}

void g() {
  const some::package::Outer msg =
      PARSE_TEXT_PROTO(
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
