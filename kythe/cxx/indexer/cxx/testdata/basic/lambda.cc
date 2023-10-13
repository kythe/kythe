//- @"[]() {}" defines LambdaFn
//- LambdaFn.node/kind function
auto fn = []() {};

//- @"[]() -> int { return 0; }" defines LambdaFn2
//- LambdaFn2.node/kind function
auto fn2 = []() -> int { return 0; };

void outer() {
  int i = 0;
  //- @"[&i]() {}" defines LambdaFn3
  //- LambdaFn3.node/kind function
  auto fn3 = [&i]() {};
}
