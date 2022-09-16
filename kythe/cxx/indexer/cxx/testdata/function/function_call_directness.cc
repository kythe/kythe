//- @f defines/binding FnF
struct A { virtual void f() {} };
//- @"A::f()" ref/call/direct FnF
struct B : public A { void f() override { A::f(); }};
