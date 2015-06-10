// Tests basic support for function templates.
// TODO(zarko): check type of AbsCallable
//- @T defines TyvarT
//- TyvarT.node/kind absvar
template <typename T>
T
//- @id defines Abs
id(T x)
{ return x; }
//- Abs.node/kind abs
//- Abs callableas AbsCallable
//- AbsCallable.node/kind callable
//- IdFun childof Abs
//- IdFun.node/kind function
//- IdFun.complete definition
//- Abs param.0 TyvarT
//- IdFun typed IdFunT
//- IdFunT.node/kind tapp
//- IdFunT param.1 TyvarT
//- IdFunT param.2 TyvarT
