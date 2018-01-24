// Tests that aggregate initialization references struct.

//- @S defines/binding StructS
struct S {};

template <typename...T>
void fn(T&&...);

void f() {
  //- @S ref StructS
  auto s = S{};

  //- @S ref StructS
  fn(S{});
}
