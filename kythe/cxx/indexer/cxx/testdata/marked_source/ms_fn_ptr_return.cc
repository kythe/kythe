// We emit reasonable markup for functions returning function pointers.

class S {
 public:
  virtual float (*bam(short function_arg) const)(int function_ptr_arg) = 0;
};

void f(const S* s) {
//- @"s->bam(0)" ref/call Fn
  (void)(s->bam(0));
}

//- Fn code FCRoot
//- FCRoot child.0 FCTypeLhs
//- FCRoot child.1 FCIdentBox
//- FCRoot child.2 FCParams
//- FCRoot child.3 FCTypeRhs
//- FCTypeLhs.kind "TYPE"
//- FCTypeLhs.pre_text "float (*"
//- FCIdentBox child.0 FCContext
//- FCContext child.0 FCContextIdent
//- FCContext.kind "CONTEXT"
//- FCContextIdent.pre_text "S"
//- FCIdentBox child.1 FCIdent
//- FCIdent.pre_text "bam"
//- FCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- FCTypeRhs.kind "TYPE"
//- FCTypeRhs.pre_text " const)(int function_ptr_arg)"
