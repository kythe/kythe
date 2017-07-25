// Tests the behavior of alias definitions combined with tycons.
//- @ptr_alias defines/binding PtrAlias
//- @int ref IntType
//- IntType.node/kind tbuiltin
using ptr_alias = int*;
//- @alias_alias defines/binding AliasAlias
//- @ptr_alias ref PtrAlias
using alias_alias = ptr_alias;
//- PtrAlias aliases PtrIntType
//- PtrIntType param.0 vname("ptr#builtin", "", "", "", "c++")
//- PtrIntType param.1 vname("int#builtin", "", "", "", "c++")
//- AliasAlias aliases PtrAlias
