// We index type alias templates.

//- @U defines/binding TyvarU
template<typename U>
//- @T defines/binding AliasTemplate
//- @U ref TyvarU
using T = U;

//- AliasTemplate.node/kind abs
//- TyvarU.node/kind absvar
//- AliasTemplate param.0 TyvarU
//- Alias childof AliasTemplate
//- Alias.node/kind talias
