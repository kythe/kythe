//- @FOO defines/binding SFOO
//- SFoo.node/kind constant
const FOO: &'static str = "hi";

fn foo() {
  //- @FOO ref SFOO
  FOO;
}
