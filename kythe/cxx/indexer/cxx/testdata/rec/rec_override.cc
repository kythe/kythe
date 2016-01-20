// We index overrides.
//- @f defines/binding CF
class C { virtual void f() { } };
//- @f defines/binding DF
//- DF overrides CF
class D : public C { void f() override { } };
