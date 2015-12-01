// Checks that the indexer emits file nodes with content.
#include "a.h"
//- vname("", "", "", "kythe/cxx/indexer/cxx/testdata/basic/file_content.cc",
//-   "").node/kind file
//- vname("", "", "", "kythe/cxx/indexer/cxx/testdata/basic/a.h", "")
//-   .node/kind file
//- vname("", "", "", "kythe/cxx/indexer/cxx/testdata/basic/a.h", "")
//-   .text "#ifndef A_H_\n#define A_H_\n#endif  // A_H_"