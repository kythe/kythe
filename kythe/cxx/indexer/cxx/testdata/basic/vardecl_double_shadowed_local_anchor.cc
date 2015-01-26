// Checks that the indexer finds and emits nodes for local variables.
//- VarNode.node/kind variable
//- VarNode named vname("x:0:0:foo#n", "", "", "", "c++")
//- VarNode2.node/kind variable
//- VarNode2 named vname("x:0:1:0:foo#n", "", "", "", "c++")
//- VarNode3.node/kind variable
//- VarNode3 named vname("x:0:1:1:0:foo#n", "", "", "", "c++")
//- VarNode4.node/kind variable
//- VarNode4 named vname("x:0:2:1:0:foo#n", "", "", "", "c++")
void foo() {                         // foo[0]
//- @x defines VarNode
  int x;                             // foo[0][0]
  {                                  // foo[0][1]
//- @x defines VarNode2
    int x;                           // foo[0][1][0]
    {                                // foo[0][1][1]
//- @x defines VarNode3
      int x;                         // foo[0][1][1][0]
    }
    {                                // foo[0][1][2]
//- @x defines VarNode4
      int x;                         // foo[0][1][2][0]
    }
  }
}
