// Checks that we don't emit dangling callables.
class C { void f(); };
//- @f defines/binding CFImpl
//- CFImpl callableas CFImplCallable
//- CFImplCallable.node/kind callable
void C::f() { }
