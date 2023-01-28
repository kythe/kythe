// Checks that DEFINE_int64 creates a flag.
#include "gflags.h"
//- @defnflag defines/binding FlagNodeDecl
//- FlagNodeDecl.node/kind google/gflag
//- FlagNodeDecl.complete incomplete
DECLARE_int64(defnflag);
//- @defnflag defines/binding FlagNode
//- FlagNode.node/kind google/gflag
//- FlagNode.complete definition
//- FlagNodeDecl completedby FlagNode
DEFINE_int64(defnflag, 0, "adef");
//- @FLAGS_defnflag ref FlagNode
//- @FLAGS_defnflag ref FlagVar
//- FlagVar.node/kind variable
long y = FLAGS_defnflag;
