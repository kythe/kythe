// Checks that we can follow substitutions one level deep.
#define M1(a,b) ((a) + (b))
int f() {
//- @x defines VarX
//- @y defines VarY
  int x = 0, y = 1;
//- @x ref VarX
//- @y ref VarY
  return M1(x, y);
}
