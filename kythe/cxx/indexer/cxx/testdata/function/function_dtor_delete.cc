// We index `delete Foo` as a call to Foo's dtor.
class C {
 public:
  //- @"~C" defines/binding Dtor
  ~C() { }
};
void f(C* c) {
  //- @"delete c" ref/call Dtor
  delete c;
}
