// This test exercises the 'declare module' syntax.

// declare module with a quoted string defines the module at
// the given path, so any symbols within in should be scoped to the
// appropriate VName.
//- @"'foo/bar'" defines/binding ModNamespace
//- ModNamespace.node/kind record
//- ModNamespace.subkind namespace
//- ModNamespace.complete definition
//- @"'foo/bar'" defines/binding ModValue
//- ModValue.node/kind package
//- ModDef defines ModValue
//- ModDef.loc/start @^"declare"
declare module 'foo/bar' {
  //- @x defines/binding _X1=vname(_, _, _, "foo/bar", _)
  let x;
  //- ModDef.loc/end @$"}"
}

//- @foobar defines/binding FooBarModule
//- FooBarModule.node/kind record
declare module foobar {
  //- @x defines/binding _X2=vname(_, _, _, "testdata/declare_module", _)
  let x;
}

//- @"'incomplete'" defines/binding IncompleteMod
//- IncompleteMod.node/kind record
//- IncompleteMod.subkind namespace
//- IncompleteMod.complete incomplete
declare module 'incomplete';
