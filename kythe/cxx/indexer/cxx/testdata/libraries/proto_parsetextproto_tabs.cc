// Checks that the content of proto string literals is indexed.
#include "message.proto.h"
#include "parsetextproto.h"

class string;

int main() {
  const some::package::Outer msg =
      ::proto2::contrib::parse_proto::ParseTextProtoOrDieAt(
          //- LiteralInner.node/kind anchor
          //- LiteralInner.loc/start @^:25"in"
          //- LiteralInner.loc/end @$:25"ner"
          //- LiteralInner ref/call InnerAccessor

          //- LiteralMyInt.node/kind anchor
          //- LiteralMyInt.loc/start @^:26"my_"
          //- LiteralMyInt.loc/end @$:26"int"
          //- LiteralMyInt ref/call MyIntAccessor

          //- LiteralMyString.node/kind anchor
          //- LiteralMyString.loc/start @^:28"my_"
          //- LiteralMyString.loc/end @$:28"string"
          //- LiteralMyString ref/call MyStringAccessor

          " inner {"
	  "  my_int: 3\n"
          " }"
          "	my_string: 'blah'",
          false, __FILE__, __LINE__);
}
