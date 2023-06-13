// Checks that completion edges are properly recorded for specializations.

//- @C defines/binding TemplateTS
template <typename T, typename S> class C;
//- @C defines/binding TemplateT
template <typename T> class C<int, T>;
//- @C defines/binding Template
template <> class C<int, float>;

//- @C defines/binding TemplateTSDefn
//- TemplateTS completedby TemplateTSDefn
template <typename T, typename S> class C { };

//- @C defines/binding TemplateTDefn
//- TemplateT completedby TemplateTDefn
template <typename T> class C<int, T> { };

//- @C defines/binding TemplateDefn
//- Template completedby TemplateDefn
template <> class C<int, float> { };
