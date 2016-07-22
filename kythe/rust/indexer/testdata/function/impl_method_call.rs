//- @dummy defines/binding Fdummy
fn dummy() {}

trait Foo {
  fn bar();
}

struct Baz;

impl Foo for Baz {
  //- @bar defines/binding FFoo_bar
  fn bar() {
    //- @dummy ref Fdummy
    //- @"dummy()" ref/call Fdummy
    //- @"dummy()" childof FFoo_bar
    dummy();
  }
}

