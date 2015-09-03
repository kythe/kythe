// We handle refs to static member function template specs in member expressions
// Needs -ignore_dups (function pointer type ref is doubly emitted)
//- @C defines/binding StructC
struct C { };
//- @f defines/binding MemberF
struct S { template <typename T> static int f(void) { return 0; } };
S s;
//- @C ref StructC
//- @"f<C>" ref TAppFC
//- TAppFC param.0 MemberF
//- TAppFC param.1 StructC
int i = (*&s.f<C>)();
