class A {
  //- @#0member defines/binding Member
  //- Member code MemberCode
  //- MemberCode child.0 MemberContext
  //- MemberContext.pre_text "A"
  //- MemberCode child.1 MemberSpace
  //- MemberSpace.pre_text " "
  //- MemberCode child.2 MemberName
  //- MemberName.pre_text "member"
  //- MemberCode child.3 MemberTy
  //- MemberTy.post_text "string"
  //- MemberCode child.4 MemberEq
  //- MemberEq.pre_text " = "
  //- MemberCode child.5 MemberInit
  //- MemberInit.pre_text "'member'"
  member = 'member';

  //- @#0staticmember defines/binding StaticMember
  //- StaticMember code StaticMemberCode
  //- StaticMemberCode child.0 StaticMemberContext
  //- StaticMemberContext.pre_text "A"
  //- StaticMemberCode child.1 StaticMemberSpace
  //- StaticMemberSpace.pre_text " "
  //- StaticMemberCode child.2 StaticMemberName
  //- StaticMemberName.pre_text "staticmember"
  //- StaticMemberCode child.3 StaticMemberTy
  //- StaticMemberTy.post_text "string"
  //- StaticMemberCode child.4 StaticMemberEq
  //- StaticMemberEq.pre_text " = "
  //- StaticMemberCode child.5 StaticMemberInit
  //- StaticMemberInit.pre_text "'staticmember'"
  static staticmember = 'staticmember';
}

let b = {
  //- @bprop defines/binding BProp
  //- BProp code BPropCode
  //- BPropCode child.0 BPropName
  //- BPropName.pre_text "bprop"
  //- BPropCode child.1 BPropTy
  //- BPropTy.post_text "number"
  //- BPropCode child.2 BPropEq
  //- BPropEq.pre_text " = "
  //- BPropCode child.3 BPropInit
  //- BPropInit.pre_text "1"
  bprop: 1,
};

interface C {
  //- @cprop defines/binding CProp
  //- CProp code CPropCode
  //- CPropCode child.0 CPropContext
  //- CPropContext.pre_text "C"
  //- CPropCode child.1 CPropSpace
  //- CPropSpace.pre_text " "
  //- CPropCode child.2 CPropName
  //- CPropName.pre_text "cprop"
  //- CPropCode child.3 CPropTy
  //- CPropTy.post_text "string"
  cprop: string;
}
