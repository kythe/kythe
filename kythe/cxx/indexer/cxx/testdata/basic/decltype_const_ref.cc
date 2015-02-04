// Checks that the result of deducing `decltype` types is recorded.
int z;
//- @x defines VarX
//- VarX typed ConstRefIntType
const int &x = z;
//- @x ref VarX
//- @decltype ref ConstRefIntType
decltype(x) y = z;
