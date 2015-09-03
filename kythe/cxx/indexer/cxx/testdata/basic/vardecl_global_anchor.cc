// Checks that the indexer finds and emits nodes for global variables.
//- @x defines/binding VarNode
//- VarNode.node/kind variable
//- VarNode named vname("x#n", "", "", "", "c++")
int x;
