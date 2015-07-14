// We don't record calls to destructors when objects go out of scope.
class C {
 public:
  //- @"~C" defines CDtor
  //- CDtor callableas CDtorC
  ~C() { }
};
void f() {
  //- !{Anchor ref/call CDtorC}
  C c;
}
