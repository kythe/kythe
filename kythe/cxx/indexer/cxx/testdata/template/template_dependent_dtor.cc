// Checks that we index explicit calls to a dependent dtor.
//- @T defines/binding TyvarT
template <typename T>
class C {
  T *t;
  void f() {
    //- @"t->~T" ref/call LookupTDtor2C
    //- LookupTDtor2 callableas LookupTDtor2C
    //- LookupTDtor2.node/kind lookup
    //- LookupTDtor2 param.0 TyvarT
    //- LookupTDtor2.text "#dtor"
    t->~T();
    //- @"delete t" ref/call LookupTDtorC
    //- LookupTDtor callableas LookupTDtorC
    //- LookupTDtor.node/kind lookup
    //- LookupTDtor param.0 TyvarT
    //- LookupTDtor.text "#dtor"
    delete t;
  }
};
