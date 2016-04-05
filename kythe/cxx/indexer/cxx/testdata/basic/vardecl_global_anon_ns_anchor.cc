// Checks that the indexer finds and emits nodes for global variables in
// anonymous namespaces.
//- @x defines/binding VarNode
//- VarNode.node/kind variable
//- @namespace defines/binding Namespace
//- VarNode childof Namespace
namespace { int x; }
