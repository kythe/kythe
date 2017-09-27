//- @foo defines/binding FooFn
constexpr int foo() { return 1; }

//- @foo ref FooFn
//- @"foo()" ref/call FooFn
static_assert(foo() == 1, "");

template <int N> struct T {};

//- @foo ref FooFn
//- @"foo()" ref/call FooFn
T<foo()> t;
