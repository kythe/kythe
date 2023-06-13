// Checks that typename template parameters are properly recorded.
template
//- @X defines/binding TvX
<typename X>
//- @C defines/binding Abs
class C {};
//- C tparam.0 TvX
//- TvX.node/kind tvar
