/**
 * @fileoverview This test verifies that importing modules declared in
 * `.d.ts` filed via `declare module` produces correct ref/imports.
 */

//- @"'foo/bar'" ref/imports FooBarSlashModule
import * as foobarslash from 'foo/bar';

//- @"'foobar'" ref/imports FooBarModule
import * as foobar from 'foobar';

//- @"'goog:foo'" ref/imports GoogFooModule
import * as googfoo from 'goog:foo';
