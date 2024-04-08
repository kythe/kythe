#include "gflags.h"

//- _File=vname("", Corpus, _, _, "").node/kind file

//- @boolflag defines/binding BoolFlag
//- BoolFlag.node/kind google/gflag
//- BoolFlag named BoolFlagName=vname("boolflag", Corpus, "", "", "flag")
//- BoolFlagName.node/kind name
DEFINE_bool(boolflag, false, "b");

//- @stringflag defines/binding StringFlag
//- StringFlag.node/kind google/gflag
//- StringFlag named StringFlagName=vname("stringflag", Corpus, "", "", "flag")
//- StringFlagName.node/kind name
DEFINE_string(stringflag, "five", "f");
