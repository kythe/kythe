// Checks that the indexer finds and emits nodes for tuple struct initialization.
//- @Foo defines/binding SFoo
//- SFoo.node/kind record
struct Foo(u32);

fn foo() {
  //- @Foo ref SFoo
  let _ = Foo(1);
}
