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
  // TODO: - @age defines/binding Age
  //- @obj defines/binding _Obj
  //- @x ref X
  let obj = {age: x};

  // TODO: - @age ref Age
  //- @destructure defines/binding Destructure
  let {age: destructure} = obj;

  //- @destructure ref Destructure
  destructure;
}

// TODO: test array destructuring.
// It currently doesn't work because we need to alter the tsconfig to bring
// in [Symbol.iterator] I think.

// Verify we don't crash on "undefined", which is special in that it is
// allowed to refer to nothing.
let undef = undefined;
