// Test #if directive.
//- @M0 defines M0
#define M0 1
//- @M0 ref/expands M0
#if M0 == 1
#endif
