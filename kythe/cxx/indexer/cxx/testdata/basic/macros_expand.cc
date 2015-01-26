// Tests that we record macro expansions only when they are at real locations.
#define M0 int x;
#define M1 M0
//- @M2 defines M2
#define M2 M1
//- @M2 ref/expands M2
M2
