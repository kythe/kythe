#pragma kythe_claim
// This is a multiplexed test file. The default file will be named
// test.cc.
#include "a.h"
//- @C=vname(_,"bundle","","test.cc",_).node/kind anchor
//- vname("","bundle","","test.cc","").node/kind file
#define C macroc
#example a.h
#pragma kythe_claim
// This is now a header file called a.h.
//- @A=vname(_,"bundle","","a.h",_).node/kind anchor
//- vname("","bundle","","a.h","").node/kind file
#define A macroa
