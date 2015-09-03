// Checks that we index operator overload members.
class C {
 public:
  //- @"operator+" defines/binding OperatorPlus
  //- OperatorPlus callableas OperatorPlusC
  //- OperatorPlus named vname("OO#Plus:C#n",_,_,_,_)
  int operator+(int q) { return 0; }
};
int a(C& c) {
  //- @"c +" ref/call OperatorPlusC
  return c + 1;
}
