// Checks that the result of deducing `decltype(auto)` types is recorded.
//- @x defines/binding VarX
//- VarX typed ConstIntType
//- @int ref IntType
const int x = 42;
// NB: "auto" doesn't refer to anything here; it's just taking up space.
//- @v defines/binding VarV
//- VarV typed ConstIntType
//- @decltype ref ConstIntType
decltype(auto) v = x;
//- @auto ref IntType
auto w = x;
