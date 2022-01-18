// We index type alias templates.

//- @U defines/binding TyvarU
template<typename U>
//- @T defines/binding T
//- @U ref TyvarU
using T = U;

//- TyvarU.node/kind tvar
//- T tparam.0 TyvarU
//- Alias.node/kind talias
