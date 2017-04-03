// Checks that single-variable specialization decls are recorded.
template
<typename T>
//- @C defines/binding CDecl
class C;

template
<>
//- @C defines/binding PCDecl
class C
<int>;

//- PCDecl specializes TAppNomCInt
//- TAppNomCInt.node/kind tapp
//- TAppNomCInt param.0 NominalC
//- IncompleteC childof CDecl
//- IncompleteC.node/kind record
//- IncompleteC.complete incomplete
//- PCDecl.node/kind record
//- PCDecl.complete incomplete