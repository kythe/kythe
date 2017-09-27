//- @B defines/binding StructB
struct B {};

//- @decltype ref StructB
decltype(auto) DecltypeAutoDeducedReturnType(const B* b) {
  return *b;
};
