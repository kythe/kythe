// Tests the behavior of multiple levels of template arguments.
//- @T defines/binding AbsT
template <typename T>
struct X {
//- @S defines/binding AbsS
  template<typename S>
  struct Y {
//- @U defines/binding AliasU
    using U = T;
  };
};
//- AliasU aliases AbsT
