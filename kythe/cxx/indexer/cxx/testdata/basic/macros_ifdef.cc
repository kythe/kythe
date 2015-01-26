// Tests #ifdef guards.
//- @M0 defines M0
//- M0 named Name0
#define M0 1
//- @M0 ref/queries M0
#ifdef M0
#endif
//- @M0 ref/queries M0
#ifndef M0
#endif
#undef M0
//- @M0 ref/queries Name0
#ifdef M0
#endif
//- @M0 ref/queries Name0
#ifndef M0
#endif
