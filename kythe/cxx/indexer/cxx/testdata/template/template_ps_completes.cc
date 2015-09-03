// Checks that completion edges are properly recorded for specializations.

//- @C defines/binding TemplateTS
template <typename T, typename S> class C;
//- @C defines/binding TemplateT
template <typename T> class C<int, T>;
//- @C defines/binding Template
template <> class C<int, float>;

//- @C completes/uniquely TemplateTS
template <typename T, typename S> class C { };

//- @C completes/uniquely TemplateT
template <typename T> class C<int, T> { };

//- @C completes/uniquely Template
template <> class C<int, float> { };
