// We don't record calls to destructors when objects go out of scope.
class C {
 public:
  //- @"~C" defines/binding CDtor
  ~C() { }
};
void f() {
  //- !{Anchor ref/call CDtor}
  C c;
}
