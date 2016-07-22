//- @FOO defines/binding SFOO
//- SFoo.node/kind variable
static FOO: &'static str = "hi";

fn foo() {
  //- @FOO ref SFOO
  FOO;
}
