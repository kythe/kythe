export {}

//- @x defines/binding X
//- X.node/kind variable
let x = 3;
//- @x ref X
x++;

// Check that we index variable initializers:
//- @x ref X
let y = x + 3;

// Check indexing through object literal values.
{
  //- @obj defines/binding Obj
  //- @x ref X
  let obj = {age: x};
}

// Verify we don't crash on "undefined", which is special in that it is
// allowed to refer to nothing.
let undef = undefined;
