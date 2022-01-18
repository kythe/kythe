// We point at the correct abs nodes in overrides when aliasing is on.
//- @f defines/binding CF
//- @C defines/binding ClassC
//- CF childof ClassC
//- !{CF overrides/root _}
template <typename T> class C { virtual void f() { } };
//- @f defines/binding DF
//- DF overrides TAppCF
//- DF overrides/root TAppCF
//- TAppCF.node/kind tapp
//- TAppCF param.0 CF
class D : public C<int> { void f() override { } };
//- @f defines/binding EF
//- EF overrides DF
//- EF overrides/root TAppCF
class E : public D { void f() override { } };
