// Checks that we index conversion functions.
class C {
 public:
  //- @operator defines/binding OperatorInt
  operator int() { return 42; }
};
int f(C &c) {
  //- @"c" ref/call OperatorInt
  return c;
}
