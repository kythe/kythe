// Checks the behavior of explicit class template instantiations under aliasing.
using ExternalDef = int;
//- @C defines/binding TemplateC
template <typename T> struct C {
  using X = ExternalDef;
};
//- @C ref TemplateC
//- @C defines/binding ExplicitC
template struct C<int>;
//- ExplicitC specializes TAppCInt
//- ExplicitC instantiates TAppCInt
//- TAppCInt param.0 TemplateC
//- ExplicitC.node/kind record
//- ExternalDefAlias childof ExplicitC
//- ExternalDefAlias.node/kind talias
//- SomeA defines/binding ExternalDefAlias
//- SomeA.node/kind anchor
