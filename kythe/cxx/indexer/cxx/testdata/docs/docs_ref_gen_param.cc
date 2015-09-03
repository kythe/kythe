// We generate cromulent references to function parameters.
///- @a ref/doc ParamA
///- @b ref/doc ParamB
/// Adds `a` to `b`.
///- @a defines/binding ParamA
///- @b defines/binding ParamB
int f(int a, int b) {
  return a + b;
}
//- goal_prefix should be ///-
