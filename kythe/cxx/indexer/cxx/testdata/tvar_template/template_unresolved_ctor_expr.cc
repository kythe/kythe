// Checks that we handle type-dependent maybe-ctor callish sites.
//   see also CXXUnresolvedConstructExpr.
//- @T defines/binding TyvarT
template<typename T, typename A1>
//- @make_a defines/binding MakeA
//- MakeA.node/kind function
inline T make_a(const A1& a1) {
  //- CallAnchor=@"T(a1)" ref/call LookupCtorT
  //- CallAnchor childof MakeA
  //- LookupCtorT.node/kind lookup
  //- LookupCtorT.text "#ctor"
  //- LookupCtorT param.0 TyvarT
  return T(a1);
}
