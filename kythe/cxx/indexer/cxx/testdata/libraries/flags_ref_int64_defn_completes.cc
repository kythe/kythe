// Checks that DEFINE_int64 creates a flag.
#include "gflags.h"
//- @defnflag defines FlagNodeDecl
//- FlagNodeDecl.node/kind google/gflag
//- FlagNodeDecl.complete incomplete
//- FlagNodeDecl named FlagName
DECLARE_int64(defnflag);
//- @defnflag defines FlagNode
//- FlagNode.node/kind google/gflag
//- FlagNode.complete definition
//- FlagNode named FlagName
//- @defnflag completes/uniquely FlagNodeDecl
DEFINE_int64(defnflag, 0, "adef");
//- @FLAGS_defnflag ref FlagNode
//- @FLAGS_defnflag ref FlagVar
//- FlagVar.node/kind variable
long y = FLAGS_defnflag;
