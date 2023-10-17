//- @outer defines/binding Outer
//- Outer.node/kind function
void outer() {
  //- @"[]() {}" childof Outer
  auto fn = []() {};
}
