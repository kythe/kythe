class A {
  //- @#0member defines/binding Member
  //- Member code MemberCode
  //- MemberCode child.0 MemberContext
  //- MemberContext.pre_text "(property)"
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
  //- StaticMemberContext.pre_text "(property)"
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
  //- BPropCode child.0 BPropContext
  //- BPropContext.pre_text "(property)"
  //- BPropCode child.1 BPropSpace
  //- BPropSpace.pre_text " "
  //- BPropCode child.2 BPropName
  //- BPropName.pre_text "bprop"
  //- BPropCode child.3 BPropTy
  //- BPropTy.post_text "number"
  //- BPropCode child.4 BPropEq
  //- BPropEq.pre_text " = "
  //- BPropCode child.5 BPropInit
  //- BPropInit.pre_text "1"
  bprop: 1,
};
