// Checks that we can complete a bool flag decl with stdbool.h included.
#include "stdbool.h"
#include "gflags.h"
//- @boolflag defines/binding BoolFlag
//- BoolFlag.complete definition
//- BoolFlag.node/kind google/gflag
DEFINE_bool(boolflag, true, "y");
