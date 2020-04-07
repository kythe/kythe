// Tests that aggregate initialization references struct.

//- @S defines/binding StructS
struct S {
  //- @a defines/binding FieldA
  int a;
  //- @b defines/binding FieldB
  long b;
};

//- @U defines/binding UnionU
union U {
  //- @x defines/binding FieldX
  int x;
  //- @y defines/binding FieldY
  long y;
};

template <typename...T>
void fn(T&&...);

void f() {
  //- @S ref StructS
  //- @"1" ref/init FieldA
  //- !{ _ ref/init FieldB }
  auto s = S{1};

  //- @U ref UnionU
  //- @"1" ref/init FieldX
  //- !{ _ ref/init FieldY }
  auto u = U{1};

  //- @S ref StructS
  fn(S{});
}
