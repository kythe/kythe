// Checks the behavior of explicit class template instantiations.
using ExternalDef = int;
//- @C defines/binding TemplateC
template <typename T> struct C {
  using X = ExternalDef;
};
//- @C defines/binding ExplicitC
//- @C ref ExplicitC
template struct C<int>;
//- ExplicitC.node/kind record
//- XAnchor childof/context ExplicitC
//- XAnchor.node/kind anchor
//- XAnchor defines/binding ExternalDefAlias
//- ExternalDefAlias.node/kind talias
