// Checks that we can complete a string flag decl.
#include "flags_string.h"
//- StringFlagDeclAnchor defines/binding StringFlagDecl
//- StringFlagDecl.complete incomplete
//- StringFlagDecl.node/kind google/gflag
//- @stringflag defines/binding StringFlag
//- StringFlag.complete definition
//- StringFlag.node/kind google/gflag
//- @stringflag completes StringFlagDecl
//- StringFlagDecl completedby StringFlag
DEFINE_string(stringflag, "gnirts", "rtsgni");
//- @FLAGS_stringflag ref StringFlag
//- @FLAGS_stringflag ref FlagVar
//- FlagVar.node/kind variable
auto s = FLAGS_stringflag;
