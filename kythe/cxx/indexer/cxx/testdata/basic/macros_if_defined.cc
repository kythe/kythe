// Test #if defined() guards.
//- @M0 defines/binding M0
#define M0 1
//- @M0 ref/queries M0
#if defined(M0)
#endif
#undef M0
//- !{@M0 ref/queries M0}
#if defined(M0)
#endif
//- @M0 defines/binding OtherM0
#define M0 1
//- @M0 ref/queries OtherM0
//- !{@M0 ref/queries M0}
#if defined(M0)
#endif
