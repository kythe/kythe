// Tests support for implicit specializations of function templates.
//- @id defines AbsId
template <typename T> T id(T x) { return x; }
//- @id ref IntSpecId
int y = id(42);
//- @id ref FloatSpecId
float z = id(42.0f);
//- IntSpecId specializes TAppAbsIdInt
//- FloatSpecId specializes TAppAbsIdFloat
//- TAppAbsIdInt param.0 AbsId
//- TAppAbsIdInt param.1 vname("int#builtin",_,_,_,_)
//- TAppAbsIdFloat param.0 AbsId
//- TAppAbsIdFloat param.1 vname("float#builtin",_,_,_,_)
