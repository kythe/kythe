//- @S defines/binding StructS
struct S {};

//- @U defines/binding StructU
struct U { S s; };

//- @kA defines/binding VarA
//- VarA typed LambdaA
//- @S ref StructS
//- @U ref StructU
//- @p defines/binding ArgPa
auto kA  = [](S p) -> U {
  //- @p ref ArgPa
  return {p};
};

// Test that identical lambdas produce unique results.
//- @kB defines/binding VarB
//- VarB typed LambdaB
//- !{ VarB typed LambdaA }
//- @S ref StructS
//- @U ref StructU
//- @p defines/binding ArgPb
auto kB  = [](S p) -> U {
  //- @S ref StructS
  //- @U ref StructU
  //- @p defines/binding ArgPinner
  return [](S p) -> U {
    //- @p ref ArgPinner
    //- !{ @p ref ArgPa }
    //- !{ @p ref ArgPb }
    return {p};
    //- @p ref ArgPb
    //- !{ @p ref ArgPa }
  }(p);
};

void enclose() {
  //- @S ref StructS
  //- @U ref StructU
  //- @p defines/binding ArgPc
  auto c = [](S p) -> U {
    //- @p ref ArgPc
    return {p};
  };

  //- @S ref StructS
  //- @U ref StructU
  //- @p defines/binding ArgPanon
  [](S p) -> U {
    //- @p ref ArgPanon
    return {p};
  };
}
