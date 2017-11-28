// This is identical to module.ts, but exporting a function instead of a value.
//- Mod=vname("module", _, _, "testdata/module_with_function", _).node/kind record

// The first 'export' statement in the module is tagged as defining
// the module.
//- @"export" defines/binding Mod
export function f() {}
