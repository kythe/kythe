// Checks that we can complete a bool flag decl.
#include "flags_bool.h"
//- BoolFlagDeclAnchor defines/binding BoolFlagDecl
//- BoolFlagDecl.complete incomplete
//- BoolFlagDecl.node/kind google/gflag
//- @boolflag defines/binding BoolFlag
//- BoolFlag.complete definition
//- BoolFlag.node/kind google/gflag
//- BoolFlagDecl completedby BoolFlag
DEFINE_bool(boolflag, true, "y");
