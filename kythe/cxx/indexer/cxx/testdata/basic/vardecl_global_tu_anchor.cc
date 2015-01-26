// Checks that the indexer finds and emits nodes for global static variables.
//- @x defines VarNode
//- VarNode.node/kind variable
//- VarNode named vname("x#n", "", "", "", "c++")
static int x;
