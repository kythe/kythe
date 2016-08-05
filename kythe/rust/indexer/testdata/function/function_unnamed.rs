//- @one defines/binding FOne
fn one() -> i32 {
  1
}

//- @two defines/binding FTwo
fn two() -> i32 {
  2
}

fn foo() {
  //- @one ref FOne
  //- @two ref FTwo
  //- !{ foo ref/call bar }
  (if true { one } else { two })();
}
