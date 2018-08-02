// ref/call anchors should cover the expression triggering the conversion.
struct T {
//- @T defines/binding TCharCtor
  T(const char*) {}
//- @T defines/binding TIntCtor
  T(int) {}
};
void c(T);
void fn() {
  //- @"\"xyz\"" ref/call TCharCtor
  c("xyz");
  //- @"1 + 2 + 4" ref/call TIntCtor
  c(1 + 2 + 4);
}
