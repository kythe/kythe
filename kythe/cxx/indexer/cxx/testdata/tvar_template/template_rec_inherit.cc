// Verifies that we emit a reference to template base classes.
//- @C defines/binding TemplateC
template <typename T> class C {};
//- @C ref CInstInt
//- CInstInt instantiates TAppCInt
//- TAppCInt param.0 TemplateC
struct S : C<int> {};
