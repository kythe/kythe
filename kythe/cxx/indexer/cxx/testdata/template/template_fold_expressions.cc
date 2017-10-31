// Verify that we parse and reference templates with C++17 fold expressions.

//- @Args defines/binding ArgsParam
template <typename... Args>
//- @Args ref ArgsParam
//- @args defines/binding ArgsVar
auto Sum(Args... args){
  //- @args ref ArgsVar
  return (args + ...);
}
