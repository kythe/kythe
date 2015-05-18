// Checks that DECLARE_int64 creates a flag.
#include "gflags.h"
//- @declflag defines FlagNode
//- FlagNode.node/kind google/gflag
//- FlagNode.complete incomplete
DECLARE_int64(declflag);
//- @FLAGS_declflag ref FlagNode
//- @FLAGS_declflag ref FlagVar
//- FlagVar.node/kind variable
long x = FLAGS_declflag;
