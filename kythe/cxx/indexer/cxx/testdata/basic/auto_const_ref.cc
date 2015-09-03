// Checks that the result of deducing qualified `auto` types is recorded.
// NB: In const auto &v = x, the "auto" is "int" if x typed const &int.
//- @x defines/binding VarX
//- VarX typed IntType
int x = 1;
//- @v defines/binding VarV
//- VarV typed RefConstIntType
//- @auto ref IntType
const auto &v = x;
//- @y defines/binding VarY
//- VarY typed RefConstIntType
const int &y = x;
