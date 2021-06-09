package tappcorpus

// When --use_compilation_corpus_as_default is enabled in the go indexer, tapp
// nodes should use the compilation unit's corpus rather than the empty corpus.

//- @x defines/binding VarX=vname(_,"kythe",_,_,"go")
//- VarX typed IntSlice=vname(_,"kythe",_,_,"go")
//- IntSlice.node/kind tapp
var x []int
