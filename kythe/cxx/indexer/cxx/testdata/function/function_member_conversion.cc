// Checks that we index conversion functions.
class C {
 public:
  //- @operator defines OperatorInt
  //- OperatorInt callableas OperatorIntC
  operator int() { return 42; }
};
int f(C &c) {
  //- @"c" ref/call OperatorIntC
  return c;
}
