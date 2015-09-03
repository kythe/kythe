// Checks that the result of deducing `auto` types is recorded.
//- @v defines/binding VarV
//- VarV typed IntType
//- @auto ref IntType
auto v = 1;
//- @x defines/binding VarX
//- VarX typed IntType
int x = 1;
