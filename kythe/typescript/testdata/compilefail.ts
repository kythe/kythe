// This file exercises what happens when you have a compilation error.
// It's also imported by compilefail_import.ts.

// This is an error because 'UndefinedSymbol' is never defined.
export type Bad = UndefinedSymbol;

// But we should still index this valid part.
//- @x defines/binding X
let x = 3;
//- @x ref X
x++;
