// Tests the behavior of alias definitions combined with tycons.
//- @ptr_alias defines PtrAlias
//- @int ref IntType
using ptr_alias = int*;
//- @alias_alias defines AliasAlias
//- @ptr_alias ref PtrAlias
using alias_alias = ptr_alias;
//- PtrAlias aliases PtrIntType
//- PtrIntType param.0 vname("ptr#builtin", "", "", "", "c++")
//- PtrIntType param.1 vname("int#builtin", "", "", "", "c++")
//- AliasAlias aliases PtrAlias