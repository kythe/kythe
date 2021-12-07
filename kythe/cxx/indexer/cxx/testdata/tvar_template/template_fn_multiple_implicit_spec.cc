// Tests support for implicit specializations of function templates.
//- @id defines/binding AbsId
template <typename T> T id(T x) { return x; }
//- @id ref TAppAbsIdInt
int y = id(42);
//- @id ref TAppAbsIdFloat
float z = id(42.0f);
//- IntSpecId specializes TAppAbsIdInt
//- IntSpecId.node/kind function
//- FloatSpecId specializes TAppAbsIdFloat
//- FloatSpecId.node/kind function
//- TAppAbsIdInt param.0 AbsId
//- TAppAbsIdInt param.1 vname("int#builtin",_,_,_,_)
//- TAppAbsIdFloat param.0 AbsId
//- TAppAbsIdFloat param.1 vname("float#builtin",_,_,_,_)
