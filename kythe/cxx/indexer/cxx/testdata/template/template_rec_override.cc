// We index overrides that come from templates.
// Note that we override C<int>::f, not \T.C<T>::f.
//- @f defines/binding CF
//- @C defines/binding TemplateC
//- CF childof CInstInt
//- CInstInt instantiates TAppCInt
//- TAppCInt param.0 TemplateC
template <typename T> class C { virtual void f() { } };
//- @f defines/binding DF
//- DF overrides CF
class D : public C<int> { void f() override { } };
