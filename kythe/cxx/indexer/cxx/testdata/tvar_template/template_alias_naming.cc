// Other flavors of template arguments don't confuse aliasing.

//- @C defines/binding ClassC
//- ClassC.node/kind record
//- @f defines/binding FnF
//- FnF childof ClassC
template <void*> class C { public: void f() { } };
void g() { typedef C<nullptr> I; I i; i.f(); }
