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
//- FCRoot child.0 FCIdentBox
//- FCIdentBox child.0 FCContext
//- FCContext child.0 FCContextIdent
//- FCContext.kind "CONTEXT"
//- FCContextIdent.pre_text "S"
//- FCIdentBox child.1 FCIdent
//- FCIdent.pre_text "bam"
//- FCRoot child.1 FCParams
//- FCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
