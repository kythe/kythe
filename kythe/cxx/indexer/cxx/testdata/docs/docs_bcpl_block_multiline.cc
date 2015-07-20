/// Multiline BCPL-flavored block comments are indexed.
///- Doc.loc/start 0
///- Doc documents ClassC
///- Doc.loc/end @$c
/// doc
class C { };
///- ClassC.node/kind record
//- goal_prefix should be ///-
