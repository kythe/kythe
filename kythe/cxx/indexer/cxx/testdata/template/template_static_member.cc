// We handle refs to static member function template specs in member expressions
// Needs -ignore_dups (function pointer type ref is doubly emitted)
//- @C defines/binding StructC
struct C { };
//- @f defines/binding MemberF
struct S { template <typename T> static int f(void) { return 0; } };
S s;
//- @C ref StructC
//- @f ref TAppFC
//- @"s.f<C>()" ref/call TAppFC
//- TAppFC param.0 MemberF
//- TAppFC param.1 StructC
//- Imp instantiates TAppFC
//- Imp.node/kind function
void qqq() { s.f<C>(); }
