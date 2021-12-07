// Checks that we don't draw spurious childof edges when aliasing is on.
template <typename T>
//- @C defines/binding AbsC
//- ClassC childof AbsC
//- @F defines/binding CiF  // aliased.
//- CiF childof ClassC
struct C { void F() { } };

//- CiF instantiates TAppCiFNil
//- TAppCiFNil param.0 CiF
void g() { C<int> ci; ci.F(); }

//- !{ CiF childof AbsC }  // The bug in question.
