// Checks that field references in a function are blamed on that function.

//- @f defines/binding FieldF
struct S { int f; };

//- @f defines/binding FnF
void f() {
  S s;
  //- @f ref/writes FieldF
  //- @f childof FnF
  s.f = 3;
}
