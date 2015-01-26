// Tests support for implicit specializations of function templates.
//- @id defines AbsId
template <typename T> T id(T x) { return x; }
//- @id ref SpecId
int y = id(42);
//- SpecId callableas SpecIdCallable
//- SpecIdCallable.node/kind callable
//- SpecId.node/kind function
//- SpecId specializes TAppAbsInt
//- TAppAbsInt.node/kind tapp
//- TAppAbsInt param.0 AbsId
//- AbsId.node/kind abs
//- TAppAbsInt param.1 vname("int#builtin",_,_,_,_)
