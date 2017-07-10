// Checks that the content of proto string literals is indexed.
#include "message.proto.h"
#include "parsetextproto.h"

class string;

int main() {
  //- @inner defines/binding InnerVar
  some::package::Inner inner;

  //- @inner ref InnerVar
  //- @my_int ref MyIntAccessor
  //- @"inner.my_int()" ref/call MyIntAccessor
  inner.my_int();

  const some::package::Outer msg =
      PARSE_TEXT_PROTO(
          //- LiteralInner.node/kind anchor
          //- LiteralInner.loc/start @^:33"in"
          //- LiteralInner.loc/end @$:33"ner"
          //- LiteralInner ref/call InnerAccessor

          //- LiteralMyInt.node/kind anchor
          //- LiteralMyInt.loc/start @^:34"my_"
          //- LiteralMyInt.loc/end @$:34"int"
          //- LiteralMyInt ref/call MyIntAccessor

          //- LiteralMyString.node/kind anchor
          //- LiteralMyString.loc/start @^:36"my_"
          //- LiteralMyString.loc/end @$:36"string"
          //- LiteralMyString ref/call MyStringAccessor

          " inner {"
          "  my_int: 3\n"
          " }"
          " my_string: 'blah'");
}
