// Verify that we emit references for C++17 pack expansion using declarations.

struct A {
  //- @Call defines/binding ACallFn
  void Call(const A&);
};

struct B {
  //- @Call defines/binding BCallFn
  void Call(const B&);
};

template <typename... Ts>
struct S : Ts... {
  //- @Call ref/implicit ACallFn
  //- @Call ref/implicit BCallFn
  using Ts::Call...;
};

void f() {
  S<A, B> c;
  //- @Call ref ACallFn
  //- @"c.Call(A{})" ref/call ACallFn
  c.Call(A{});
  //- @Call ref BCallFn
  //- @"c.Call(B{})" ref/call BCallFn
  c.Call(B{});
}
