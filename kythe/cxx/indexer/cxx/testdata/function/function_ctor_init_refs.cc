// We ref member variables from the init list.
class A {
 public:
  A(int a) {}
};

class C {
  //- @ivar ref/writes IVar
  //- @avar ref/writes AVar
  C() : ivar(88), avar(A(20)) { }

  //- @ivar defines/binding IVar
  int ivar;
  //- @avar defines/binding AVar
  A avar;
};
