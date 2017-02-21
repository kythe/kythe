export {}

//- @x defines/binding X
let x: any;

// ".zzz" is a reference off an "any", so it will have no Symbol
// and there's no useful cross-referencing to be done.  This test
// passes if there's no crash.
//- @x ref X
x.zzz;
