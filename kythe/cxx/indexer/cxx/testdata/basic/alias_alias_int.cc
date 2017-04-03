// Tests the behavior of alias definitions.
//- @builtin_alias defines/binding BuiltinAlias
//- @int ref IntType
using builtin_alias = int;
//- @alias_alias defines/binding AliasAlias
//- @builtin_alias ref BuiltinAlias
using alias_alias = builtin_alias;
//- BuiltinAlias.node/kind talias
//- AliasAlias.node/kind talias
//- BuiltinAlias aliases IntType
//- AliasAlias aliases BuiltinAlias
//- BuiltinAlias aliases vname("int#builtin", "", "", "", "c++")
