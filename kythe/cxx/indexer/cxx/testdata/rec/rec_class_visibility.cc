//- @C defines/binding StructC
class C {
  //- @f defines/binding F
  //- F.visibility "private"
  void f();

  //- @v1 defines/binding V1
  //- V1.visibility "private"
  int v1;

 public:
  //- @g defines/binding G
  //- G.visibility "public"
  void g();

  //- @v2 defines/binding V2
  //- V2.visibility "public"
  int v2;

 protected:
  //- @h defines/binding H
  //- H.visibility "protected"
  void h();

  //- @h2 defines/binding H2
  //- H2.visibility "protected"
  void h2() { }

  //- @v3 defines/binding V3
  //- V3.visibility "protected"
  int v3;
};

//- @D defines/binding StructD
struct D {
  //- @j defines/binding J
  //- J.visibility "public"
  void j();
};
