// Checks that repeated structured types have their constituents marked.
template <typename T> class C { };
//- @D defines/binding ClassD
class D { };
//- @D ref ClassD
C<D> x;
//- @D ref ClassD
C<D> y;
