// Tests basic support for function templates.
//- @T defines/binding TT
//- TT.node/kind tvar
template <typename T>
T
//- @id defines/binding IdFun
id(T x)
{ return x; }
//- IdFun.node/kind function
//- IdFun.complete definition
//- IdFun tparam.0 TT
//- IdFun typed IdFunT
//- IdFunT.node/kind tapp
//- IdFunT param.1 TyvarT
//- IdFunT param.2 TyvarT
