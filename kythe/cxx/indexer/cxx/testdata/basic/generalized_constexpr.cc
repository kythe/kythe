// C++14's generalized constexpr, including loops.
//- @Fib defines/binding FibFn
//- FibFn.node/kind function
//- @count defines/binding CountArg
constexpr auto Fib(int count) {
  //- @count ref CountArg
  if (count < 3) return 1;  // Just to add some complexity!
  int a = 1, b = 1;
  //- @count ref CountArg
  for (int i = 1; i < count; ++i) {
    b += a;
    a = b - a;
  }
  return a;
}

//- @Fib ref FibFn
static_assert(Fib(6) == 8, "");
