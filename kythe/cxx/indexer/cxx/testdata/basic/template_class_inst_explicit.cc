// Checks the behavior of explicit class template instantiations.
using ExternalDef = int;
//- @C defines TemplateC
template <typename T> struct C {
  using X = ExternalDef;
};
template struct C<int>;
//- ExplicitC.node/kind record
//- XAnchor childof ExplicitC
//- XAnchor.node/kind anchor
//- XAnchor defines ExternalDefAlias
//- ExternalDefAlias.node/kind talias