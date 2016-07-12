// Checks that the indexer finds and emits nodes for local variables.
//- VarNode.node/kind variable
fn foo() {
//- @x defines/binding VarNode
  let x: u32;
}
