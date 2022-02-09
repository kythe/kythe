// We index dependent base classes.
//- @T defines/binding TyvarT
template <typename T>
//- @A defines/binding ClassA
//- ClassA extends/public TyvarT
//- ClassA extends/private DepTS
//- DepTS.node/kind lookup
//- DepTS param.0 TyvarT
//- DepTS.text S
class A : public T, private T::S { };
