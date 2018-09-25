//- @FwdEnum defines/binding FwdDecl
//- FwdEnum.node/kind sum
//- FwdEnum.complete incomplete
//- FwdEnum.subkind enumClass
enum class FwdEnum;

//- @Box defines/binding BoxClass
struct Box {
  //- @efwd defines/binding EFwdDecl
  //- EFwdDecl childof BoxClass
  //- @FwdEnum ref FwdDecl
  //- EFwdDecl typed TAppPtrFwdEnum
  //- TAppPtrFwdEnum param.1 vname("FwdEnum#n#t",_,_,_,_)
  FwdEnum* efwd;
};
