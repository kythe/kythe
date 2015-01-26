// Tests the behavior of multiple levels of template arguments.
//- @T defines AbsT
template <typename T>
struct X {
//- @S defines AbsS
  template<typename S>
  struct Y {
//- @U defines AliasU
    using U = T;
  };
};
//- AliasU aliases AbsT
