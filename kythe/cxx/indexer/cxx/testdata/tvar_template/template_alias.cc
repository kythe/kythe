// We index type alias templates.

//- @U defines/binding TyvarU
template<typename U>
//- @T defines/binding AliasTemplate
//- @U ref TyvarU
using T = U;

//- AliasTemplate.node/kind abs
//- TyvarU.node/kind tvar
//- T childof AliasTemplate
//- T tparam.0 TyvarU
//- Alias childof AliasTemplate
//- Alias.node/kind talias
