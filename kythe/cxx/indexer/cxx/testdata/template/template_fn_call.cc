template <typename T> void f() {}

//- @"A" defines/binding ClsA
struct A {};

void g() {
  //- @"A" ref ClsA
  f<A>();
}
