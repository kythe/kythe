// We emit reasonable markup for functions and their parameters.
//- @fn1 defines/binding Fn1
//- Fn1 code FCRoot
//- FCRoot child.0 FCVoid
//- FCVoid.kind "TYPE"
//- FCVoid.pre_text void
//- FCRoot child.1 FCSpace
//- FCSpace.pre_text " "
//- FCRoot child.2 FCIdent
//- FCIdent.pre_text fn1
//- FCRoot child.3 FCParams
//- FCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- FCParams.pre_text "("
//- FCParams.post_text ")"
//- FCParams.post_child_text ", "
//- @arg1 defines/binding Arg1
//- Arg1 code ACRoot
//- ACRoot child.0 ACType
//- ACType.kind "TYPE"
//- ACType.pre_text "int"
//- ACRoot child.1 ACSpace
//- ACSpace.pre_text " "
//- ACRoot child.2 ACIdent
//- ACIdent child.0 ACContext
//- ACContext.kind "CONTEXT"
//- ACContext.post_child_text "::"
//- ACContext child.0 ACContextIdent
//- ACContextIdent.kind "IDENTIFIER"
//- ACContextIdent.pre_text "fn1"
//- ACIdent child.1 ACIdentToken
//- ACIdentToken.kind "IDENTIFIER"
//- ACIdentToken.pre_text "arg1"
void fn1(int arg1);
