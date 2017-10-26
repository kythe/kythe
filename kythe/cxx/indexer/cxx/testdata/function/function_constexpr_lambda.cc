// We index C++17 constexpr lambdas

//- @k defines/binding ConstK
constexpr int k = 10;

void f() {
  auto fn = []() constexpr {
    //- @k ref ConstK
    return k;
  };

  //- @"fn()" ref/call LambdaFn
  int vars[fn()];
}
