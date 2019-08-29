export {};

//- @obj defines/binding Obj
const obj = {
  a: ''
};

// Verify that a reference within a 'typeof' clause refers to the value.
//- @obj ref Obj
const user: typeof obj = {
  a: ''
};
