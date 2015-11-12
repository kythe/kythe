// Checks that we can deal with non-ASCII characters in comments.
// This character maps to one UTF-16 code unit. If the indexer ignores
// the differences between byte offset and code unit offset, then the binding
// below will be defined from "r f" (because code units and byte offsets
// go -2 out of sync).
// —
//- @"foo" defines/binding VarFoo
var foo = 1;
