// File-level comments are recorded.

// And class-level comments are too.
class C {};

//- MainFile=vname("","","","kythe/cxx/indexer/cxx/testdata/docs/docs_file_level.cc","").node/kind file
//- MainDoc documents MainFile
//- MainDoc.text " File-level comments are recorded."
//- ClassC.node/kind record
//- ClassDoc documents ClassC
//- ClassDoc.text " And class-level comments are too."
