// Checks that references to a complete template class point to the right node.
template
<typename X>
//- @C defines/binding Abs
class C {};

//- @U defines/binding AliasNode
using U =
C<int>;

//- AliasNode aliases TApp
//- TApp param.0 Abs
//- Abs.node/kind abs
//- ClassC childof Abs
//- ClassC.node/kind record
