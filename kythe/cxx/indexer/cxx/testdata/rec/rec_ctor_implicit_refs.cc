// Implicit refs to fields aren't drawn over complete constructor names.
struct Z { };
struct S {
  //- !{@S ref VarX}
  //- @y ref/writes VarY
  //- !{@S ref VarZ}
  //- _ ref/call ZCtor
  //- ZCtor childof ZType
  S() : y() { }
  //- @x defines/binding VarX
  //- VarX typed ZType
  Z x;
  //- @y defines/binding VarY
  Z y;
  //- @z defines/binding VarZ
  Z z = Z {};
};
