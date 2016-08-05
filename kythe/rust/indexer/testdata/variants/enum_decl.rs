// Checks that the indexer finds and emits nodes for enum declarations.
//- @Foo defines/binding EFoo
//- EFoo.node/kind sum
enum Foo {
  //- @A defines/binding VA
  //- VA childof EFoo
  //- VA.node/kind record
  A,
  //- @B defines/binding VB
  //- VB childof EFoo
  //- VB.node/kind record
  B(i32),
  //- @C defines/binding VC
  //- VC childof EFoo
  //- VC.node/kind record
  C {
    x: i32
  }
}
