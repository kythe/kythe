// Verifies that Kythe references auto template type parameters
// as it would any other non-type template parameter.

//- @N defines/binding TemplateParamN
template <auto N>
struct S {
  //- @#0N ref TemplateParamN
  //- @#1N ref TemplateParamN
  //- @values defines/binding FieldArr
  decltype(N) values[N];
};

void f() {
 S<10> ints;
 S<'a'> chars;
}
