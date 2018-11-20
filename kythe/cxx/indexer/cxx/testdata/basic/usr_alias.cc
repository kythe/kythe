// We index USRs for aliases.

//- @Int defines/binding Int
//- IntUsr /clang/usr Int
//- IntUsr.node/kind clang/usr
using Int = int;

//- @Short defines/binding Short
//- ShortUsr /clang/usr Short
//- ShortUsr.node/kind clang/usr
typedef short Short;

struct S {
//- @Float defines/binding Float
//- FloatUsr /clang/usr Float
//- FloatUsr.node/kind clang/usr
  using Float = float;
};

void f() {
  //- @Double defines/binding Double
  //- !{_ /clang/usr Double}
  using Double = double;
}
