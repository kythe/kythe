// Checks that function defns complete function decls.
#include "void_f.h"
//- @f defines/binding FSDecl
void f();
//- @f defines/binding FSDefn
//- @f completes FHDecl
//- @f completes/uniquely FSDecl
//- FSDecl completedby FSDefn
//- FHDecl completedby FSDefn
void f() { }
//- FHAnchor=vname(_,_,_,"kythe/cxx/indexer/cxx/testdata/function/void_f.h","c++")
//-    defines/binding FHDecl
