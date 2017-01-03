//- @"A" defines/binding ClsA
struct A {};

void f() {
  //- @"A" ref ClsA
  new A;
}
