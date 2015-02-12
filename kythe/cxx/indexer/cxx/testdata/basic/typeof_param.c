// Checks that parameters of typeof()-declared functions are uniquely named.
//- @int ref IntTy
//- @float ref FloatTy
void foo(int a, float b);
//- @bar defines FnBar
//- FnBar param.0 AnonInt AnonInt typed IntTy
//- FnBar param.1 AnonFloat AnonFloat typed FloatTy
extern typeof(foo) bar;
