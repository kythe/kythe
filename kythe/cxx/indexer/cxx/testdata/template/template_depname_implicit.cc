// Checks that we can refer to implicit dependent fields.
//- @A defines AbsA
template <typename T> class A { };
//- @T defines TyvarT
template <typename T> class C {
//- @Dep ref Dep
//- Dep param.0 At
//- At.node/kind tapp
//- At param.0 AbsA
//- At param.1 TyvarT
//- @T ref TyvarT
  void f() { A<T>::Dep(); }
};
