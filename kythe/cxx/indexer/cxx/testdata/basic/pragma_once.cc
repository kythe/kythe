//- NativeAnchor=@"\"kythe/cxx/indexer/cxx/testdata/basic/pragma_once.h\"" ref/includes NativeHeader
#include "kythe/cxx/indexer/cxx/testdata/basic/pragma_once.h"
//- MappedAnchor=@"\"kythe_indexer_test/pragma_once.h\"" ref/includes MappedHeader
#include "kythe_indexer_test/pragma_once.h"

// Ideally, these files would have the same VName, but since they don't
// ensure that they are different, as expected, for now.
//- !{ NativeAnchor ref/includes MappedHeader }
//- !{ MappedAnchor ref/includes NativeHeader }
