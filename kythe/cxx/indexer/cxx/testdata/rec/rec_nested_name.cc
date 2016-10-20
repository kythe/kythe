// We emit references to nodes in nested names.
//- @U defines/binding StructU
struct U {
//- @S defines/binding StructS
  struct S {
//- @T defines/binding StructT
    struct T {
//- @f defines/binding FnF
      static int f();
    };
  };
};

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @f ref FnF
int v = U::S::T::f();
