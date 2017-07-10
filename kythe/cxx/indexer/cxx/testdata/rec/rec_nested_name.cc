// We emit references to nodes in nested names.
//- @U defines/binding StructU
struct U {
//- @S defines/binding StructS
  struct S {
//- @T defines/binding StructT
    struct T {
//- @f defines/binding FnF
      static int f();
//- @d defines/binding DataD
      static int d;
    };
  };
};

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @f ref FnF
int v = U::S::T::f();

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @f completes/uniquely FnF
int U::S::T::f() { }


//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @d completes/uniquely DataD
int U::S::T::d = 1;
