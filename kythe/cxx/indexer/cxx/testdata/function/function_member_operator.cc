// Checks that we index operator overload members.
class C {
 public:
  //- @"operator+" defines/binding OperatorPlus
  int operator+(int q) { return 0; }
};
int a(C& c) {
  //- @"c + 1" ref/call OperatorPlus
  return c + 1;
}
