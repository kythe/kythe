// Tests that we index instances of partial specializations of template vars.
//- @v defines/binding Prim
template <typename T, typename S, typename V> T v;
template <typename U>
//- @v defines/binding V
U v<int, U, long>;
//- @v ref Spec
float w = v<int, float, long>;
//- Spec specializes TAppPrim
//- Spec instantiates TAppV
//- TAppPrim param.0 Prim
//- TAppPrim param.1 vname("int#builtin",_,_,_,_)
//- TAppPrim param.2 vname("float#builtin",_,_,_,_)
//- TAppPrim param.3 vname("long#builtin",_,_,_,_)
//- TAppV param.0 V
//- TAppV param.1 vname("float#builtin",_,_,_,_)
