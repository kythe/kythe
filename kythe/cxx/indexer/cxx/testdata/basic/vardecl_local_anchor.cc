// Checks that the indexer finds and emits nodes for local variables.
//- VarNode.node/kind variable
void foo() {
//- @x defines/binding VarNode
  int x;
}
