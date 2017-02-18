// Namespaces are named the same in any file.
#pragma kythe_claim
#include "foo.h"
//- @foo ref NamespaceFoo
namespace foo {
}
#example foo.h
#pragma kythe_claim
//- @foo ref NamespaceFoo
namespace foo {
}
