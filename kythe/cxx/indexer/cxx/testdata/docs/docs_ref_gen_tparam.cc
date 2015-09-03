// We generate cromulent references to template parameters.
///- @A ref/doc TyvarA
///- @B ref/doc TyvarB
/// Pairs `A` with `B`.
///- @A defines/binding TyvarA
///- @B defines/binding TyvarB
template <typename A, typename B>
class C { };
//- goal_prefix should be ///-
