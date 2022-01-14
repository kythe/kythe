// Checks various uses of `auto` from the Standard.

//- @auto ref GenericLambda
//- GenericLambda.node/kind record
auto glambda =
//- @int ref IntType
//- @auto ref ImplicitTyvar
//- ImplicitTyvar.node/kind tvar
    [](int i, auto a) { return i; };

//- @auto ref IntType
auto x = 5;

//- @auto ref IntType
const auto *v = &x, u = 6;

//- @double ref DoubleType
double q = 0.0;

//- @auto ref DoubleType
static auto y = 0.0;

// 'auto' here is just a syntactic wart.
// auto f() -> int;

//- @auto ref DoubleType
auto g() { return 0.0; }
