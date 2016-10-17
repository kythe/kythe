// We index overrides.
//- @f defines/binding CF
//- !{ CF overrides/root _ }
class C { virtual void f() { } };
//- @f defines/binding DF
//- DF overrides CF
//- DF overrides/root CF
class D : public C { void f() override { } };
//- @f defines/binding EF
//- EF overrides DF
//- EF overrides/root CF
class E : public D { void f() override { } };
