// Checks that we can apply semantics to functions.
#pragma kythe_metadata "semantic.meta"
//- @foo defines/binding FnFoo
void foo();

void bar() {
  //- @"foo()" ref/writes vname(gsig, gcorp, groot, gpath, glang)
  //- @foo ref FnFoo
  //- @"foo()" ref/call FnFoo
  foo();
}
