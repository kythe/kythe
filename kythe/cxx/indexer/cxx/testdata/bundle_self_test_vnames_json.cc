// Check to see if test_vnames.json does the right thing.
#pragma kythe_claim
#include "acorpus_aroot_apath.h"
#include "bcorpus__bpath.h"
#include "_croot_cpath.h"
#include "long_path.h"
//- @main_v=vname(_,"bundle","","test.cc","c++")
//-     childof vname("","bundle","","test.cc","")
#define main_v macromainv


#example acorpus_aroot_apath.h
#pragma kythe_claim
//- @a_v=vname(_,"acorpus","aroot","apath.h","c++") childof
//-     vname("","acorpus","aroot","apath.h","")
#define a_v macroav


#example bcorpus__bpath.h
#pragma kythe_claim
//- @b_v childof vname("","bcorpus","","bpath.h","")
#define b_v macrobv


#example _croot_cpath.h
#pragma kythe_claim
//- @c_v childof vname("","","croot","cpath.h","")
#define c_v macrocv

#example long_path.h
#pragma kythe_claim
//- @long_path_v childof vname("",
//-     "012345678901234567890123456789012345678901234567890123456789",
//-     "012345678901234567890123456789012345678901234567890123456789",
//-     "012345678901234567890123456789012345678901234567890123456789", "")
#define long_path_v macrolongpathv
