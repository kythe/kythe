// Tests the `using Name = ty` syntactic form.
//- @alias defines TypeAlias
//- @"int" ref IntType
//- TypeAlias.node/kind talias
//- TypeAlias aliases IntType
//- TypeAlias aliases vname("int#builtin", "", "", "", "c++")
//- TypeAlias named vname("alias#n", "", "", "", "c++")
using alias = int;
