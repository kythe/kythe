// We emit reasonable markup for functions in namespaces.
namespace ns {
  namespace {
//- @f defines/binding FnF
//- FnF code FCRoot
//- FCRoot child.2 FnFIdent
//- FnFIdent child.0 FnFContext
//- FnFIdent child.1 FnFToken
//- FnFToken.kind "IDENTIFIER"
//- FnFToken.pre_text "f"
//- FnFContext.kind "CONTEXT"
//- FnFContext child.0 FnFNsNs
//- FnFNsNs.kind "IDENTIFIER"
//- FnFNsNs.pre_text "ns"
//- FnFContext child.1 FnFAnonNS
//- FnFAnonNS.kind "IDENTIFIER"
//- FnFAnonNS.pre_text "(anonymous namespace)"
    void f();
  }
}
