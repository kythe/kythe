//- @B defines/binding StructB
struct B {};

//- @auto ref StructB
auto SimpleDeducedReturnType() { return B{}; }
