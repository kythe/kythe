// Checks that we get decorations on references inside macros.
#define DEFINE(a, b) a { b }
template <typename T>
struct A {
  A(int i) {}
};
//- @B defines/binding B
struct B { };
//- @foo defines/binding Foo
int foo(int x) { return x + 1; }
//- @a defines/binding VarA
//- VarA typed TAppAB
//- @B ref B
//- @"A<B>" ref TAppAB
//- @foo ref Foo
DEFINE(A<B> a, foo(
//- @foo ref Foo
  foo(0)));
