// File-level comments with includes are recorded.
// This file includes some other files.
#include "docs_file_level_includes_a.h"
#include "docs_file_level_includes_b.h"
//- MainFile=vname("","","","kythe/cxx/indexer/cxx/testdata/docs/docs_file_level_includes.cc","").node/kind file
//- HeaderA=vname("","","","kythe/cxx/indexer/cxx/testdata/docs/docs_file_level_includes_a.h","").node/kind file
//- HeaderB=vname("","","","kythe/cxx/indexer/cxx/testdata/docs/docs_file_level_includes_b.h","").node/kind file
//- MainDoc documents MainFile
//- ADoc documents HeaderA
//- BDoc documents HeaderB
//- MainDoc.text " File-level comments with includes are recorded.\n This file includes some other files."
//- ADoc.text " This is an include file."
//- BDoc.text " This is also an include file."
