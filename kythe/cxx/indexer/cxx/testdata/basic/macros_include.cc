// Checks that we emit file references for includes.
//- @"\"a.h\"" ref/includes vname(_,_,_,
//-                               "kythe/cxx/indexer/cxx/testdata/basic/a.h",
//-                               "")
#include "a.h"
