// Tests that references to ps class templates go to the ps via `specializes`.
//- @V defines/binding VPrim
template <typename T, typename S> struct V { S m; };
//- @V defines/binding VInt
//- VInt specializes TAppVIntInt
template <typename S> struct V<int, S> { S mm; };
//- @x defines/binding VarX
//- VarX typed TAppVIntInt
V<int, int> x;
