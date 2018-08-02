// ref/call anchors should cover the expression triggering the conversion.
struct T {
//- @T defines/binding TCharCtor
  T(const char*) {}
//- @T defines/binding TIntCtor
  T(int) {}
};
void c(T);
void fn() {
  // The anchor for implicit conversions should span the entire expression.
  //- String ref/call TCharCtor
  //- String.loc/start @$"("
  //- String.loc/end @$z
  c("xyz");
  //- Addition ref/call TIntCtor
  //- Addition.loc/start @^"1"
  //- Addition.loc/end @$"4"
  c(1 + 2 + 4);
}
