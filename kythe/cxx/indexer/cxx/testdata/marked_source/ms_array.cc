// Array-typed variables have proper MarkedSource.
namespace n {
//- @x defines/binding VarX
//- VarX code MsX
//- MsX child.0 MsTypePart0
//- MsX child.1 MsBoxCxtId
//- MsX child.2 MsTypePart1
//- MsTypePart0.kind "TYPE"
//- MsTypePart0.pre_text "char "
//- MsBoxCxtId child.0 CxtNs
//- CxtNs.kind "CONTEXT"
//- CxtNs child.0 Ns
//- Ns.kind "IDENTIFIER"
//- Ns.pre_text "n"
//- MsBoxCxtId child.1 Id
//- Id.pre_text "x"
//- Id.kind "IDENTIFIER"
//- MsTypePart1.kind "TYPE"
//- MsTypePart1.pre_text "[]"
char x[] = {1, 2};
}
