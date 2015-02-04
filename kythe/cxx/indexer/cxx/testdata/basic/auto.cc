// Checks that the result of deducing `auto` types is recorded.
//- @v defines VarV
//- VarV typed IntType
//- @auto ref IntType
auto v = 1;
//- @x defines VarX
//- VarX typed IntType
int x = 1;
