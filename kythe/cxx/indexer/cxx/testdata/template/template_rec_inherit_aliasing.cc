// Verifies that we emit a reference to template base classes.
//- @C defines/binding TemplateC
template <typename T> class C {};
//- @C ref TemplateC
//- @"C<int>" ref TAppCInt
//- TAppCInt param.0 TemplateC
//- TAppCInt param.1 Int
//- @int ref Int
struct S : C<int> {};
