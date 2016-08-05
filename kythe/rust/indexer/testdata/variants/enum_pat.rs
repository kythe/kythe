// Checks that the indexer finds and emits nodes for enum destructuring.
//- @Foo defines/binding EFoo
//- EFoo.node/kind sum
enum Foo {
  //- @A defines/binding VA
  A,
  //- @B defines/binding VB
  B(i32),
  //- @C defines/binding VC
  C {
    x: i32
  }
}

fn main() {
  use Foo::*;
  //- @B ref VB
  let loc = B(3);
  match loc {
    //- @A ref VA
    A => (),
    //- @B ref VB
    B(x) => (),
    //- @C ref VC
    C { x: y } => (),
  }
}
