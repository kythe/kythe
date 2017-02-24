// We emit reasonable markup for functions in records.
struct S {
  struct {
//- @f defines/binding FnF
//- FnF code FCRoot
//- FCRoot child.2 FCIdent
//- FCIdent child.0 FCContext
//- FCContext child.0 FCS
//- FCS.kind "IDENTIFIER"
//- FCS.pre_text "S"
//- FCContext child.1 FCAnon
//- FCAnon.pre_text "(anonymous struct)"
//- FCIdent child.1 FCToken
//- FCToken.kind "IDENTIFIER"
//- FCToken.pre_text "f"
    void f();
  } T;
};
