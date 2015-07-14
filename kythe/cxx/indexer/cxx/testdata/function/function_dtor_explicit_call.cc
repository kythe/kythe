// Checks that we index explicit destructor calls.
class C {
 public:
  //- @"~C" defines CDtor
  //- CDtor callableas CDtorC
  //- CDtor named vname("~C:C#n",_,_,_,_)
  ~C() { }
};
void F(C* c) {
  //- @"c->~C()" ref/call CDtorC
  c->~C();
}
