// We treat calls to operator++ the same way we treat primitive assignment.
struct S { S& operator++(); };
S operator--(S&);

void f() {
  //- @a defines/binding Sa
  //- @b defines/binding Sb
  S a, b;
  //- @a ref/writes Sa
  //- Sa influences Sa
  ++a;
  //- @b ref/writes Sb
  //- Sb influences Sb
  --b;
}
