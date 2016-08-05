// Checks that the indexer finds and emits nodes for unit struct initialization.
//- @Foo defines/binding SFoo
//- SFoo.node/kind record
struct Foo;

fn foo() {
  //- @Foo ref SFoo
  let _ = Foo;
}
