// Checks that we can deal with non-ASCII characters in comments.
// This character (deseret small letter yee (U+10437)) requires the use of a
// surrogate pair in UTF-16. If the indexer ignores the differences between
// byte offset and code unit offset, then the binding below will be defined
// from "r f" (because code units and byte offsets go -2 out of sync). If
// the indexer were counting in code points instead of code units, then it
// would be -3 out of sync.
// 𐐷
//- @"foo" defines/binding VarFoo
var foo = 1;
