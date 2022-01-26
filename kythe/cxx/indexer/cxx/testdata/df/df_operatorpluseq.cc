// We treat calls to operator+= the same way we treat primitive assignment.
struct S { S& operator+=(const S&); };
S& operator-=(S&, const S&);

void f() {
  //- @a defines/binding Sa
  //- @b defines/binding Sb
  //- @c defines/binding Sc
  //- @d defines/binding Sd
  S a, b, c, d;
  //- @a ref/writes Sa
  //- @b ref Sb
  //- Sb influences Sa
  //- Sa influences Sa
  //- !{Sb influences Sb}
  //- !{@b ref/writes _}
  a += b;
  //- @c ref/writes Sc
  //- @d ref Sd
  //- Sd influences Sc
  //- Sc influences Sc
  //- !{Sd influences Sd}
  //- !{@d ref/writes _}
  c -= d;
}
