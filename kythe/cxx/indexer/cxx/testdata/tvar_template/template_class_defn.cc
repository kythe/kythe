// Checks that references to a complete template class point to the right node.
template
<typename X>
//- @C defines/binding ClassC
class C {};

//- @U defines/binding AliasNode
using U =
C<int>;

//- AliasNode aliases TApp
//- TApp param.0 ClassC
//- ClassC.node/kind record
