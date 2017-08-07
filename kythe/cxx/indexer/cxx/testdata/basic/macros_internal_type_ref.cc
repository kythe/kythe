#define _REALLY_PASTE(a, b) a##b
#define _PASTE(a, b) _REALLY_PASTE(x##a##y, z)
#define DEFINE(a, b, c) auto _PASTE(c, __LINE__) = f<c>(f<a>(0))
//- @A defines/binding A
class A { };
//- @C defines/binding C
class C { };
template <typename T> int f(int x) { return x + 1; }
//- @A ref A
//- @C ref C
DEFINE(A, B, C);
