
void fn() {
  //- @outer defines/binding OuterVar
  int outer = 0;
  //- @var defines/binding VarArg
  //- @capture defines/binding CaptureBinding
  //- @outer ref OuterVar
  auto lambda = [capture=&outer] (const auto& var) {
    //- @capture ref CaptureBinding
    //- @var ref VarArg
    return capture + var;
  };
};
