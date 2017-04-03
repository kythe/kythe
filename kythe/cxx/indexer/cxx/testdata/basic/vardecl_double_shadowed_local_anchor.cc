// Checks that the indexer finds and emits nodes for local variables.
//- VarNode.node/kind variable
//- VarNode2.node/kind variable
//- VarNode3.node/kind variable
//- VarNode4.node/kind variable
void foo() {                         // foo[0]
//- @x defines/binding VarNode
  int x;                             // foo[0][0]
  {                                  // foo[0][1]
//- @x defines/binding VarNode2
//- !{@x defines/binding VarNode}
    int x;                           // foo[0][1][0]
    {                                // foo[0][1][1]
//- @x defines/binding VarNode3
//- !{@x defines/binding VarNode2
//-   @x defines/binding VarNode}
      int x;                         // foo[0][1][1][0]
    }
    {                                // foo[0][1][2]
//- @x defines/binding VarNode4
//- !{@x defines/binding VarNode3
//-   @x defines/binding VarNode2
//-   @x defines/binding VarNode}
      int x;                         // foo[0][1][2][0]
    }
  }
}
