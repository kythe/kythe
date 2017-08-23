void fn() {
  //- @S defines/binding StructS
  struct S {
    //- @S ref StructS
    //- @p defines/binding ArgP
    //- ArgP typed StructS
    void run(S p) {}
  };
  //- @s defines/binding VarS
  //- VarS typed StructS
  S s;
}
