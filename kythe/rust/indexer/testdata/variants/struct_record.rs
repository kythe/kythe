// Checks that the indexer finds and emits nodes for struct initialization.
//- @Foo defines/binding SFoo
//- SFoo.node/kind record
struct Foo {
  x: i32
}

fn foo() {
  //- @Foo ref SFoo
  let _ = Foo {
    x: 3
  };
}
