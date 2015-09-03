// Checks that we properly handle opening and closing quotes.
///- @foo ref/doc FnFoo
///- !{ @bar ref/doc FnBar }
///- @baz ref/doc FnBaz
/// `foo` bar `baz`
///- @bar defines/binding FnBar
int bar() { return 0; }
///- @foo defines/binding FnFoo
int foo() { return 0; }
///- @baz defines/binding FnBaz
int baz() { return 0; }
//- goal_prefix should be ///-
