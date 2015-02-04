// Check to see if test_vnames.json does the right thing.
#pragma kythe_claim
#include "acorpus_aroot_apath.h"
#include "bcorpus__bpath.h"
#include "_croot_cpath.h"
//- @main_v childof vname("","bundle","","test.cc","c++")
#define main_v macromainv


#example acorpus_aroot_apath.h
#pragma kythe_claim
//- @a_v childof vname("","acorpus","aroot","apath.h","c++")
#define a_v macroav


#example bcorpus__bpath.h
#pragma kythe_claim
//- @b_v childof vname("","bcorpus","","bpath.h","c++")
#define b_v macrobv


#example _croot_cpath.h
#pragma kythe_claim
//- @c_v childof vname("","","croot","cpath.h","c++")
#define c_v macrocv
