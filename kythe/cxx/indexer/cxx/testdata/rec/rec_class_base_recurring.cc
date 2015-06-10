// We index recurring base classes.
// TODO(zarko): below, consider making
//   ClassB extends/public InstAB
// After T27 is finished, `specializes` will become
// `instantiates`.
//- @T defines TyvarT
template <typename T>
//- @A defines AbsA
//- ClassA childof AbsA
class A {
//- @C defines AliasC
//- AliasC aliases TyvarT
  using C = T;
};

//- @B defines ClassB
//- ClassB extends/public InstAB
//- InstAB.node/kind tapp
//- InstAB param.0 AbsA
//- InstAB param.1 ClassB
//- ABInst specializes InstAB
class B
  : public A<B> { };
