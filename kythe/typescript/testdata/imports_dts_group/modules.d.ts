/** @fileoverview See docs in imports.ts */

//- @"'foo/bar'" defines/binding FooBarSlashModule
//- FooBarSlashModule.node/kind record
declare module 'foo/bar' {
  const x = 1;
}

//- @"'foobar'" defines/binding FooBarModule
//- FooBarModule.node/kind record
declare module 'foobar' {
  const x = 1;
}

//- @"'goog:foo'" defines/binding GoogFooModule
//- GoogFooModule.node/kind record
declare module 'goog:foo' {
  const x = 1;
}
