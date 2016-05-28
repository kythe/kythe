// Checks that we index explicit destructor calls.
class C {
 public:
  //- @"~C" defines/binding CDtor
  //- CDtor named vname("~C:C#n",_,_,_,_)
  ~C() { }
};
void F(C* c) {
  //- @"c->~C()" ref/call CDtor
  c->~C();
}
