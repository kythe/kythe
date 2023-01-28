// Checks that forward declarations from headers are not ucompleted in TUs.
#include "rec_class_header_completes.h"
//- HeaderAnchor=vname(_,_,_,
//-     "kythe/cxx/indexer/cxx/testdata/rec/rec_class_header_completes.h",_)
//-  defines/binding HClassCFwd

//- @C defines/binding ClassCFwd
//- ClassCFwd.node/kind record
//- ClassCFwd.complete incomplete
//- ClassCFwd.subkind class
class C;
//- @C defines/binding ClassC
//- ClassCFwd completedby ClassC
//- HClassCFwd completedby ClassC
class C { };
//- ClassC.node/kind record
//- ClassC.complete definition
//- ClassC.subkind class
//- HClassCFwd.complete incomplete
