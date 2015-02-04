// Checks that we correctly handle headers with multiple valid transcripts.
// Needs --ignore-duplicates because of the duplicated header guards.
#define A_H_1
#include "a.h"
#include "b.h"
#undef A_H_1
#define A_H_2
#include "a.h"
#include "b.h"
#example a.h
#ifdef A_H_1
// This kythe_claim is only included in the transcript if A_H_1 is defined.
#pragma kythe_claim
//- @FOO1 defines MacroFoo
#define FOO1
#endif
#ifdef A_H_2
// Unclaimed
//- !{ @FOO2 defines MacroFoo2 }
#define FOO2
#endif

#example b.h
// This kythe_claim is included in every transcript.
#pragma kythe_claim
#ifdef A_H_1
//- @FOO3 defines MacroFoo3
#define FOO3
#endif
#ifdef A_H_2
//- @FOO4 defines MacroFoo4
#define FOO4
#endif
