// We emit reasonable markup for functions and their parameters named with a
// macro.
#define FN(x) prefix_##x
#define fn1 FN(fn1)

//- @fn1 defines/binding Fn1
//- Fn1 code FCRoot1
//- FCRoot1 child.0 FCVoid1
//- FCVoid1.kind "TYPE"
//- FCVoid1.pre_text void
//- FCRoot1 child.1 FCSpace1
//- FCSpace1.pre_text " "
//- FCRoot1 child.2 FCIdent1
//- FCIdent1.pre_text prefix_fn1
//- FCRoot1 child.3 FCParams1
//- FCParams1.kind "PARAMETER_LOOKUP_BY_PARAM"
//- FCParams1.pre_text "("
//- FCParams1.post_text ")"
//- FCParams1.post_child_text ", "
//- @arg1 defines/binding Arg1
//- Arg1 code ACRoot1
//- ACRoot1 child.0 ACType1
//- ACType1.kind "TYPE"
//- ACType1.pre_text "int"
//- ACRoot1 child.1 ACSpace1
//- ACSpace1.pre_text " "
//- ACRoot1 child.2 ACIdent1
//- ACIdent1 child.0 ACContext1
//- ACContext1.kind "CONTEXT"
//- ACContext1.post_child_text "::"
//- ACContext1 child.0 ACContextIdent1
//- ACContextIdent1.kind "IDENTIFIER"
//- ACContextIdent1.pre_text "prefix_fn1"
//- ACIdent1 child.1 ACIdentToken1
//- ACIdentToken1.kind "IDENTIFIER"
//- ACIdentToken1.pre_text "arg1"
void fn1(int arg1){}

//- @"FN(fn2)" defines/binding Fn2
//- Fn2 code FCRoot2
//- FCRoot2 child.0 FCVoid2
//- FCVoid2.kind "TYPE"
//- FCVoid2.pre_text void
//- FCRoot2 child.1 FCSpace2
//- FCSpace2.pre_text " "
//- FCRoot2 child.2 FCIdent2
//- FCIdent2.pre_text prefix_fn2
//- FCRoot2 child.3 FCParams2
//- FCParams2.kind "PARAMETER_LOOKUP_BY_PARAM"
//- FCParams2.pre_text "("
//- FCParams2.post_text ")"
//- FCParams2.post_child_text ", "
//- @arg2 defines/binding Arg2
//- Arg2 code ACRoot2
//- ACRoot2 child.0 ACType2
//- ACType2.kind "TYPE"
//- ACType2.pre_text "char"
//- ACRoot2 child.1 ACSpace2
//- ACSpace2.pre_text " "
//- ACRoot2 child.2 ACIdent2
//- ACIdent2 child.0 ACContext2
//- ACContext2.kind "CONTEXT"
//- ACContext2.post_child_text "::"
//- ACContext2 child.0 ACContextIdent2
//- ACContextIdent2.kind "IDENTIFIER"
//- ACContextIdent2.pre_text "prefix_fn2"
//- ACIdent2 child.1 ACIdentToken2
//- ACIdentToken2.kind "IDENTIFIER"
//- ACIdentToken2.pre_text "arg2"
void FN(fn2)(char arg2){}
