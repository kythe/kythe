// Tests that aggregate initialization references struct.

struct Default {
  Default();
};

//- @S defines/binding StructS
struct S {
  //- @a defines/binding FieldA
  int a;
  //- @b defines/binding FieldB
  long b;
  //- @c defines/binding FieldC
  bool c = false;
  //- @d defines/binding FieldD
  Default d;
};

//- @T defines/binding StructT
struct T : S {
  union {
    //- @j defines/binding FieldJ
    int j;
    //- @k defines/binding FieldK
    long k;
  };
  //- @l defines/binding FieldL
  int l;
  //- @m defines/binding FieldM
  char m;
};

//- @U defines/binding UnionU
union U {
  //- @x defines/binding FieldX
  int x;
  //- @y defines/binding FieldY
  long y;
};

//- @V defines/binding StructV
struct V {
  //- @s defines/binding FieldS
  S s;
  //- @t defines/binding FieldT
  T t;
};

S FromValue();

template <typename...T>
void fn(T&&...);

void f() {
  //- @S ref StructS
  //- @"1" ref/init FieldA
  //- !{ @"}" ref/init FieldB }
  //- !{ @"}" ref/init FieldC }
  //- !{ @"}" ref/init FieldD }
  auto s = S{1};

  //- @T ref StructT
  //- @"1" ref/init FieldA
  auto t = T{1};

  //- @T ref StructT
  //- @"{1,2}" ref/init StructS
  //- @"1" ref/init FieldA
  //- @"2" ref/init FieldB
  //- @"3" ref/init FieldJ
  //- @"4" ref/init FieldL
  //- @"5" ref/init FieldM
  auto full = T{{1,2},3,4,5};

  //- @U ref UnionU
  //- @"1" ref/init FieldX
  auto u = U{1};

  //- @S ref StructS
  fn(S{});

  //- !{ @"FromValue()" ref/init _ }
  S c{FromValue()};

  //- V ref StructV
  //- @"{1}" ref/init FieldS
  //- @"1" ref/init FieldA
  //- @"2" ref/init FieldT
  //- @"2" ref/init FieldA
  //- !{ @#0"}" ref/init _ }
  //- !{ @#1"}" ref/init _ }
  V v{{1}, 2};
}
