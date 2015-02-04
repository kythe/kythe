#pragma kythe_claim
// This is a multiplexed test file. The default file will be named
// test.cc.
#include "a.h"
//- AnchorC.node/kind anchor
//- AnchorC childof vname("","bundle","","test.cc","c++")
#define C macroc
#example a.h
#include "b.h"
//- !{ @A=AnchorA.node/kind anchor
//-    AnchorA childof vname("","bundle","","a.h","c++") }
#define A macroa
#example b.h
#pragma kythe_claim
//- @B=AnchorB.node/kind anchor
//- AnchorB childof vname("","bundle","","b.h","c++")
#define B macrob
