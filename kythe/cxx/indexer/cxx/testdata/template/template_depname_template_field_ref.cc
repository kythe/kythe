// Checks that we can refer to dependent fields under other templates.
//- @C defines ClassC
class C { };
//- @T defines TyvarT
template <typename T> struct S {
  T t;
  //- @f ref DepF
  //- DepF.node/kind lookup
  //- DepF.text f
  //- DepF param.0 TyvarT
  //- @C ref ClassC
  //- @"f<C>" ref TAppDepFC
  //- TAppDepFC.node/kind tapp
  //- TAppDepFC param.0 DepF
  //- TAppDepFC param.1 ClassC
  int i = t.template f<C>;
  // (f<C> should refer to a lookup of (typeof t)::f applied to C)
};
