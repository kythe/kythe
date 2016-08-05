// Checks that the indexer finds and emits nodes for struct destructuring.
//- @Foo defines/binding SFoo
//- SFoo.node/kind record
struct Foo {
  x: i32
}

fn foo() {
  //- @Foo ref SFoo
  let a = Foo {
    x: 3
  };

  //- @Foo ref SFoo
  let Foo {x: b} = a;
}
