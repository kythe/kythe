// Checks that forward declarations from headers are not ucompleted in TUs.
#include "enum_decl_ty_header_completes.h"
//- HeaderAnchor defines HEnumEFwd
//- HeaderAnchor childof vname(_,_,_,
//-     "kythe/cxx/indexer/cxx/testdata/basic/enum_decl_ty_header_completes.h",
//-     _)
//- @E defines EnumEFwd
//- EnumEFwd named EnumEName
//- EnumEName.node/kind name
//- EnumEFwd.complete complete
enum class E : short;
//- @E defines EnumE
//- @E completes/uniquely EnumEFwd
//- @E completes HEnumEFwd
enum class E : short { };
//- EnumE named EnumEName
//- EnumE typed ShortType
//- EnumE.complete definition
//- HEnumEFwd.complete complete
//- HEnumEFwd typed ShortType
//- HEnumEFwd named EnumEName