// We index dependent base classes.
//- @T defines TyvarT
template <typename T>
//- @A defines AbsA
//- ClassA childof AbsA
//- ClassA extends/public TyvarT
//- ClassA extends/private DepTS
//- DepTS.node/kind lookup
//- DepTS param.0 TyvarT
//- DepTS.text S
class A : public T, private T::S { };
