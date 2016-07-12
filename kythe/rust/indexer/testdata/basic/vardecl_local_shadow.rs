// SHOULD FAIL: Checks if indexer uses the same variable node for shadowed decls
//- VarNode.node/kind variable
fn foo() {
  //- @x defines/binding VarNode
  let x: u32;
  //- @x defines/binding VarNode
  let x: f64;
}
