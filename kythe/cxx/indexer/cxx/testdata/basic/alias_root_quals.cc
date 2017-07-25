// We emit aliases/root edges for type aliases with qualifiers.
//- @CI defines/binding CIa
//- CIa aliases CI
using CI = int* const;
//- @CVI defines/binding CVIa
//- CVIa aliases CVI
using CVI = int* const volatile;
//- @RCVI defines/binding RCVIa
//- RCVIa aliases RCVI
using RCVI = int* __restrict volatile const;
//- @T defines/binding AliasT
//- AliasT aliases TPtrInt
//- TPtrInt.node/kind tapp
using T = int*;
//- @S defines/binding AliasS
//- AliasS aliases ConstAliasT
//- ConstAliasT param.1 AliasT
//- AliasS aliases/root CI
using S = const T;
//- @U defines/binding AliasU
//- AliasU aliases VolatileAliasS
//- VolatileAliasS param.1 AliasS
//- AliasU aliases/root CVI
using U = volatile S;
//- @V defines/binding AliasV
//- AliasV aliases RestrictAliasU
//- RestrictAliasU param.1 AliasU
//- AliasV aliases/root RCVI
using V = __restrict U;
