// We index `delete Foo` as a call to Foo's dtor.
class C {
 public:
  //- @"~C" defines Dtor
  //- Dtor callableas DtorC
  ~C() { }
};
void f(C* c) {
  //- @"delete c" ref/call DtorC
  delete c;
}
