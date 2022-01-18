// Tests support for implicit specializations of function templates.
//- @id defines/binding AbsId
template <typename T> T id(T x) { return x; }
//- @id ref TAppAbsInt
int y = id(42);
//- SpecId.node/kind function
//- SpecId specializes TAppAbsInt
//- TAppAbsInt.node/kind tapp
//- TAppAbsInt param.0 AbsId
//- AbsId.node/kind function
//- TAppAbsInt param.1 vname("int#builtin",_,_,_,_)
