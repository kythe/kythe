#pragma kythe_claim
// This is a multiplexed test file. The default file will be named
// test.cc.
#include "a.h"
//- AnchorC.node/kind anchor
//- AnchorC childof vname("","bundle","","test.cc","")
#define C macroc
#example a.h
#include "b.h"
//- !{ @A=AnchorA.node/kind anchor
//-    AnchorA childof vname("","bundle","","a.h","") }
#define A macroa
#example b.h
#pragma kythe_claim
//- @B=AnchorB.node/kind anchor
//- AnchorB childof vname("","bundle","","b.h","")
#define B macrob
