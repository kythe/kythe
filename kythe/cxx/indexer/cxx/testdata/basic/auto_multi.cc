// Checks that the result of deducing `auto` types is recorded.
//- @auto ref IntType
auto x = 1;
//- @auto ref IntType
const auto *v = &x, u = 6;
