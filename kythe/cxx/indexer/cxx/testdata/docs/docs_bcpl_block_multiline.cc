/// Multiline BCPL-flavored block comments are indexed.

///- Doc documents ClassC
///- Doc.loc/start @^:8"///"
///- Doc.loc/end @$:10"doc3"
///- ClassC.node/kind record

/// doc1
/// doc2
/// doc3
class C { };
