// Tests the `using Name = ty` syntactic form.
//- @alias defines/binding TypeAlias
//- @"int" ref IntType
//- TypeAlias.node/kind talias
//- TypeAlias aliases IntType
//- TypeAlias aliases vname("int#builtin", "", "", "", "c++")
using alias = int;
