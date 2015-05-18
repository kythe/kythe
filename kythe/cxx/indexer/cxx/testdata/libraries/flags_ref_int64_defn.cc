// Checks that DEFINE_int64 creates a flag.
#include "gflags.h"
//- @defnflag defines FlagNode
//- FlagNode.node/kind google/gflag
//- FlagNode.complete definition
DEFINE_int64(defnflag, 0, "adef");
//- @FLAGS_defnflag ref FlagNode
//- @FLAGS_defnflag ref FlagVar
//- FlagVar.node/kind variable
long y = FLAGS_defnflag;
