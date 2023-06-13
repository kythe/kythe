// We index overrides that come from templates.
// Note that we override C<int>::f, not \T.C<T>::f.
//- @f defines/binding CF
//- @C defines/binding TemplateC
//- CF childof CInstInt
//- CInstInt instantiates TAppCInt
//- TAppCInt param.0 TemplateC
//- !{CF overrides/root _}
template <typename T> class C { virtual void f() { } };
//- @f defines/binding DF
//- DF overrides CF
//- DF overrides/root CF
class D : public C<int> { void f() override { } };
//- @f defines/binding EF
//- EF overrides DF
//- EF overrides/root CF
class E : public D { void f() override { } };
