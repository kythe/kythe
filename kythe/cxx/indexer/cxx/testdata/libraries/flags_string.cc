// Checks that we can complete a string flag decl.
#include "flags_string.h"
//- StringFlagDeclAnchor defines StringFlagDecl
//- StringFlagDecl.complete incomplete
//- StringFlagDecl.node/kind google/gflag
//- @stringflag defines StringFlag
//- StringFlag.complete definition
//- StringFlag.node/kind google/gflag
//- @stringflag completes StringFlagDecl
DEFINE_string(stringflag, "gnirts", "rtsgni");
//- @FLAGS_stringflag ref StringFlag
//- @FLAGS_stringflag ref FlagVar
//- FlagVar.node/kind variable
auto s = FLAGS_stringflag;
