// Checks that the indexer finds and emits nodes for local variables.
//- VarNode.node/kind variable
//- VarNode2.node/kind variable
void foo() {
//- @x defines/binding VarNode
//- !{@x defines/binding VarNode2}
  int x;
  {
//- @x defines/binding VarNode2
    int x;
  }
}
