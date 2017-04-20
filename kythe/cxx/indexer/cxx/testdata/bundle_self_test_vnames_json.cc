// Check to see if test_vnames.json does the right thing.
#pragma kythe_claim
#include "_croot_cpath.h"
#include "acorpus_aroot_apath.h"
#include "bcorpus__bpath.h"
#include "long_path.h"
//- @main_v=vname(_,"bundle","","test.cc","c++").node/kind anchor
#define main_v macromainv


#example acorpus_aroot_apath.h
#pragma kythe_claim
//- @a_v=vname(_,"acorpus","aroot","apath.h","c++").node/kind anchor
#define a_v macroav


#example bcorpus__bpath.h
#pragma kythe_claim
//- @b_v=vname(_,"bcorpus","","bpath.h","c++").node/kind anchor
#define b_v macrobv


#example _croot_cpath.h
#pragma kythe_claim
//- @c_v=vname(_,"","croot","cpath.h","c++").node/kind anchor
#define c_v macrocv

#example long_path.h
#pragma kythe_claim
//- @long_path_v=vname(_,
//-     "012345678901234567890123456789012345678901234567890123456789",
//-     "012345678901234567890123456789012345678901234567890123456789",
//-     "012345678901234567890123456789012345678901234567890123456789",
//-     "c++").node/kind anchor
#define long_path_v macrolongpathv
