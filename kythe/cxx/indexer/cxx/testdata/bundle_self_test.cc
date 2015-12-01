#pragma kythe_claim
// This is a multiplexed test file. The default file will be named
// test.cc.
#include "a.h"
//- AnchorC.node/kind anchor
//- AnchorC childof vname("","bundle","","test.cc","")
#define C macroc
#example a.h
#pragma kythe_claim
// This is now a header file called a.h.
//- @A=AnchorA.node/kind anchor
//- AnchorA childof vname("","bundle","","a.h","")
#define A macroa
