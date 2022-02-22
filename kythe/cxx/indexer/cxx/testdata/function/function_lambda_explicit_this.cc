// We index lambdas which explicitly capture `this`.

//- @S defines/binding StructS
struct S {
  void f() const {
    //- !{ @this ref StructS }
    [this] {
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
