// Checks that macros are claimed and given the correct vnames.
#pragma kythe_claim
#define A_H_ALTERNATE
#include "acorpus_aroot_apath.h"
#include "bcorpus_broot_bpath.h"

#example acorpus_aroot_apath.h
#pragma kythe_claim
//- @FOO defines MacroFoo=vname(_,"acorpus","aroot","apath.h",_)
#define FOO
//- @FOO ref/expands MacroFoo
FOO
//- @FOO undefines MacroFoo
#undef FOO

#example bcorpus_broot_bpath.h
// Unclaimed
//- !{ @BAR defines MacroBar }
//- !{ MacroBar named vname("BAR#m",_,_,_,_) }
#define BAR
//- !{ @BAR ref/expands MacroBar }
BAR
//- !{ @BAR undefines MacroBar }
#undef BAR
