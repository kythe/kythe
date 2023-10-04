// We index overrides that come from templates.
//- @f defines/binding CF
//- CF.node/kind function
//- @C defines/binding TemplateC
//- TemplateC.node/kind record
//- CF childof CInstInt
//- CInstInt.node/kind record
//- // CInstInt instantiates TAppCInt  // Not in the aliased graph.
//- TAppCInt param.0 TemplateC
template <typename T> class C { virtual void f() { } };
//- @f defines/binding DF
//- DF.node/kind function
//- DF overrides TAppCF
//- TAppCF param.0 CF
//- @C ref TemplateC
//- @"C<int>" ref TAppCInt
class D : public C<int> { void f() override { } };
