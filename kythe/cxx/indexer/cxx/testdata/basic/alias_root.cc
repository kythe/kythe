// We emit aliases/root edges for type aliases.
//- @T defines/binding AliasT
//- AliasT aliases TInt
//- AliasT aliases/root TInt
using T = int;
//- @S defines/binding AliasS
//- AliasS aliases AliasT
//- AliasS aliases/root TInt
using S = T;
//- @U defines/binding AliasU
//- AliasU aliases AliasS
//- AliasU aliases/root TInt
using U = S;
