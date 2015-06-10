// Checks that we still perform reasonably when skipping template instantations.
// Run with --index_template_instantiations=false
//- @vector defines VectorAbs
template <typename T> class vector { };
//- @ty defines TyAlias
//- TyAlias aliases Tau
//- Tau.node/kind tapp
//- Tau param.0 VectorAbs
using ty = vector<int>;
ty var;
