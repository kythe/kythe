// Tests that we index partial specializations of template variables.
//- @v defines/binding VarV
template <typename T, typename S> T v;
//- @U defines/binding TyvarU
template <typename U>
//- @v defines/binding VarPSU
U v<U, int>;
//- VarPSU.node/kind variable
//- VarPSU specializes TAppVarV
//- TAppVarV.node/kind tapp
//- TAppVarV param.0 VarV
//- TAppVarV param.1 TyvarU
//- TAppVarV param.2 vname("int#builtin",_,_,_,_)
