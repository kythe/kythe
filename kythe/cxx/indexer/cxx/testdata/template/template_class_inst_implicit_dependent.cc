// Checks the behavior of implicit class template instantiations.
//- @T defines TemplateT
template <typename C> struct T {
//- @X defines XAlias
//- XAlias aliases Lookup
//- Lookup.node/kind lookup
  using X = typename C::Y;
};
struct S { using Y = int; }; T<S>::X x;
//- ImpX specializes TAppCS
//- TAppCS.node/kind tapp
//- TAppCS param.0 TemplateT
