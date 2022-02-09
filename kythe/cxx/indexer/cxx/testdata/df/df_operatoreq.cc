// We treat calls to operator= the same way we treat primitive assignment.
struct S { S& operator=(const S&) = default; };

void f() {
  //- @a defines/binding Sa
  //- @b defines/binding Sb
  S a, b;
  //- @a ref/writes Sa
  //- @b ref Sb
  //- Sb influences Sa
  //- !{Sa influences Sa}
  //- !{@b ref/writes _}
  a = b;
}
