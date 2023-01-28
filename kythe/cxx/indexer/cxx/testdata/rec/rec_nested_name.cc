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

      //- @V defines/binding StructVDecl
      struct V;
    };
  };
};

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @V defines/binding StructVDefn
//- StructVDecl completedby StructVDefn
struct U::S::T::V {};

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @f ref FnF
int v = U::S::T::f();

//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @f defines/binding FnFDefn
//- FnF completedby FnFDefn
int U::S::T::f() { }


//- @U ref StructU
//- @S ref StructS
//- @T ref StructT
//- @d defines/binding DataDDefn
//- DataD completedby DataDDefn
int U::S::T::d = 1;
