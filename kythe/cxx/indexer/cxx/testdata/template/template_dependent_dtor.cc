// Checks that we index explicit calls to a dependent dtor.
//- @T defines/binding TyvarT
template <typename T>
class C {
  T *t;
  void f() {
    //- @"t->~T" ref/call LookupTDtor2
    //- LookupTDtor2.node/kind lookup
    //- LookupTDtor2 param.0 TyvarT
    //- LookupTDtor2.text "#dtor"
    //- @"~" ref LookupTDtor2
    t->~T();
    //- @"delete t" ref/call LookupTDtor
    //- LookupTDtor.node/kind lookup
    //- LookupTDtor param.0 TyvarT
    //- LookupTDtor.text "#dtor"
    delete t;
  }
};
