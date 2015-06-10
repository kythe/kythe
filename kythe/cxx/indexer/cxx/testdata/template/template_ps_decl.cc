// Checks that single-variable specialization decls are recorded.
template
<typename T>
//- @C defines CDecl
class C;

template
<>
//- @C defines PCDecl
class C
<int>;

//- CDecl named CName
//- PCDecl named CName
//- PCDecl specializes TAppNomCInt
//- TAppNomCInt.node/kind tapp
//- TAppNomCInt param.0 NominalC
//- NominalC named CName
//- IncompleteC childof CDecl
//- IncompleteC.node/kind record
//- IncompleteC.complete incomplete
//- PCDecl.node/kind record
//- PCDecl.complete incomplete