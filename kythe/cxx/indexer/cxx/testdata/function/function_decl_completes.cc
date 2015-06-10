// Checks that function defns complete function decls.
#include "void_f.h"
//- @f defines FSDecl
void f();
//- @f defines FSDefn
//- @f completes FHDecl
//- @f completes/uniquely FSDecl
void f() { }
//- FHAnchor defines FHDecl
//- FHAnchor childof vname(_,_,_,
//-                        "kythe/cxx/indexer/cxx/testdata/function/void_f.h",_)