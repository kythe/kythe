// Checks declaring classes with macro-derived names.
// TODO(zarko): Add macro nodes and edges. Now we'll just check that we don't
// mess up the semantics of the class decl or otherwise go wrong.
#define M C
class M;
//- ClassC.node/kind record
//- ClassC.subkind class
//- ClassC.complete incomplete