// Checks that the indexer finds and emits nodes for tuple struct destructuring.
//- @Foo defines/binding SFoo
//- SFoo.node/kind record
struct Foo(u32);

fn foo() {
  //- @Foo ref SFoo
  let a = Foo(1);
  //- @Foo ref SFoo
  let Foo(b) = a;
}
