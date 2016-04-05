// Namespaces are named the same in any file.
#pragma kythe_claim
#include "foo.h"
//- @foo defines/binding NamespaceFoo
namespace foo {
}
#example foo.h
#pragma kythe_claim
//- @foo defines/binding NamespaceFoo
namespace foo {
}
