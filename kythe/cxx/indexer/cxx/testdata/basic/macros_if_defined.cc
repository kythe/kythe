// Test #if defined() guards.
//- @M0 defines M0
//- M0 named Name0
#define M0 1
//- @M0 ref/queries M0
#if defined(M0)
#endif
#undef M0
//- @M0 ref/queries Name0
#if defined(M0)
#endif
//- @M0 defines OtherM0
#define M0 1
//- @M0 ref/queries OtherM0
#if defined(M0)
#endif
