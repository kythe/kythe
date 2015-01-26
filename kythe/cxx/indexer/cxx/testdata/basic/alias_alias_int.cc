// Tests the behavior of alias definitions.
//- @builtin_alias defines BuiltinAlias
//- @int ref IntType
using builtin_alias = int;
//- @alias_alias defines AliasAlias
//- @builtin_alias ref BuiltinAlias
using alias_alias = builtin_alias;
//- BuiltinAlias.node/kind talias
//- AliasAlias.node/kind talias
//- BuiltinAlias aliases IntType
//- AliasAlias aliases BuiltinAlias
//- BuiltinAlias aliases vname("int#builtin", "", "", "", "c++")
//- BuiltinAlias named vname("builtin_alias#n", "", "", "", "c++")
//- AliasAlias named vname("alias_alias#n", "", "", "", "c++")
//- vname("talias(alias_alias#n,talias(builtin_alias#n,int#builtin))",
//-     "", "", "", "c++")
//-   aliases vname("talias(builtin_alias#n,int#builtin)", "", "", "", "c++")
