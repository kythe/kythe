// This test exercises the 'declare module' syntax.

// declare module with a quoted string defines the module at
// the given path, so any symbols within in should be scoped to the
// appropriate VName.
declare module 'foo/bar' {
  //- @x defines/binding X1=VName(_, _, _, "foo/bar", _)
  let x;
}

declare module foobar {
  //- @x defines/binding X2=VName(_, _, _, "testdata/declare_module", _)
  let x;
}
