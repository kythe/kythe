//- @S defines/binding StructTemplateS
template <typename T> struct S {};

//- @U defines/binding StructU
struct U {};

//- @V defines/binding StructV
struct V {
  V(const S<U>&);
};

//- @S ref StructTemplateS
//- @U ref StructU
//- @p defines/binding ArgP
//- @V ref StructV
auto _ = [](S<U> p) -> V {
  //- @p ref ArgP
  return p;
};
