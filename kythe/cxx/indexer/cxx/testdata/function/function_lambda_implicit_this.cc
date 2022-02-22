// We index lambdas which implicitly capture this via either `=` or `&`.

//- @S defines/binding StructS
struct S {
  void f() const {
    [&] {
      //- @g ref MethodG
      //- @m ref FieldM
      //- !{ @g ref StructS }
      g() + m;
    };
    [=] {
      //- @g ref MethodG
      //- @m ref FieldM
      //- !{ @g ref StructS }
      g() + m;
    };
  }

  //- @g defines/binding MethodG
  int g() const;

  //- @m defines/binding FieldM
  int m;
};
