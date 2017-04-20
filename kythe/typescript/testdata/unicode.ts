// This module tests offsets when the input text contains non-ASCII text.
// It exercises the UTF-16<->UTF-8 handling.

export {}

//- @"ಠ_ಠ" defines/binding Look
let ಠ_ಠ = 'grumpy';

//- @foo defines/binding Foo
let foo = 3;

//- @"ಠ_ಠ" ref Look
//- @foo ref Foo
let sum = ಠ_ಠ + foo;
