export {};

// This top-level 'x' is here so we ensure that that 'x's in the for loops
// doesn't get the same name.
//- @x defines/binding X
//- X.node/kind variable
const x = 3;

//- @x defines/binding X2
//- X2.node/kind variable
for (const x of [1, 2]) {
  //- @x ref X2
  x;
}

//- @#0x defines/binding X3
//- X3.node/kind variable
//- @#1x ref X3
//- @#2x ref X3
for (let x = 0; x < 1; x++) {
  //- @x ref X3
  x;
}
