export {}

//- @x defines/binding X
//- X.node/kind variable
let x = 3;
//- @x ref X
x++;

// Check that we index variable initializers:
//- @x ref X
let y = x + 3;
