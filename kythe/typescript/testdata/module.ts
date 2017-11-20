// All modules define a 'module' record representing the file as a module.
// The signature is 'module' and the path is the module path (the filename
// without an extension).
//- Mod=vname("module", _, _, "testdata/module", _).node/kind record

// The first 'export' statement in the module is tagged as defining
// the module.
//- @"export" defines/binding Mod
export var value = 3;

export type MyType = string;
