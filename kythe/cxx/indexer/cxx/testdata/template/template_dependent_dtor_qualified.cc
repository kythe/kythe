// Checks that we index explicit calls to a dependent dtor with a qualifier.
//- @T defines/binding TyvarT
template <typename T>
class C {
  T *t;
  void f() {
    //- @"t->SomeId::Base::~Base" ref/call Dtor
    //- Dtor.node/kind lookup
    //- Dtor.text "#dtor"
    //- Dtor param.0 BaseInT
    //- BaseInT.node/kind lookup
    //- BaseInT.text Base
    //- BaseInT param.0 SomeId
    //- SomeId.node/kind lookup
    //- SomeId.text "SomeId"
    //- @"~" ref Dtor
    t->SomeId::Base::~Base();
    // TODO(zarko): Does it make sense for SomeId param.0 TyvarT?
    // What if instead of 'SomeId' we wrote 'T' above?
  }
};
