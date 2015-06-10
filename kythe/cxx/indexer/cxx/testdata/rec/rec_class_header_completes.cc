// Checks that forward declarations from headers are not ucompleted in TUs.
#include "rec_class_header_completes.h"
//- HeaderAnchor defines HClassCFwd
//- HeaderAnchor childof vname(_,_,_,
//-     "kythe/cxx/indexer/cxx/testdata/rec/rec_class_header_completes.h",_)
//- @C defines ClassCFwd
//- ClassCFwd named ClassCName
//- ClassCName.node/kind name
//- ClassCFwd.node/kind record
//- ClassCFwd.complete incomplete
//- ClassCFwd.subkind class
class C;
//- @C defines ClassC
//- @C completes/uniquely ClassCFwd
//- @C completes HClassCFwd
class C { };
//- ClassC named ClassCName
//- ClassC.node/kind record
//- ClassC.complete definition
//- ClassC.subkind class
//- HClassCFwd.complete incomplete
//- HClassCFwd named ClassCName
