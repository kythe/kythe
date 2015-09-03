// Checks that the indexer finds and emits nodes for global variables in
// anonymous namespaces.
//- @x defines/binding VarNode
//- VarNode.node/kind variable
//- VarNode named vname("x:@#n", "", "", "", "c++")
namespace { int x; }
