// Tests explicit specialization of function templates.
// Note that function templates do not support partial specialization.
//- @id defines/binding PrimaryTemplate
template <typename T> T id(T x) { return T(); }
//- @id defines/binding SpecTemplate
template <> int id(int x) { return x; }
//- SpecTemplate specializes TAppPTInt
//- TAppPTInt.node/kind tapp
//- TAppPTInt param.0 PrimaryTemplate
//- TAppPTInt param.1 vname("int#builtin",_,_,_,_)
