// Checks that we index explicit destructor calls.
class C {
 public:
  //- @"~C" defines/binding CDtor
  ~C() { }
};
void F(C* c) {
  //- @"c->~C()" ref/call CDtor
  c->~C();
}
