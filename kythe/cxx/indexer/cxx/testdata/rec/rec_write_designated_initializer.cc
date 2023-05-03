// Designated initializers are writes.

//- @x defines/binding FieldX
struct s { int x; };
void f() {
  //- @x ref/writes FieldX
  //- _SomeAnchor ref/init FieldX
  //- !{@x ref/init _}
  s S{.x = 4};
  //- @#0x ref/writes FieldX
  //- @#1x ref FieldX
  //- !{@#1x ref/writes FieldX}
  s T{.x = S.x};
}
