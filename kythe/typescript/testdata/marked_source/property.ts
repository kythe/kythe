class A {
  //- @#0member defines/binding Member
  //- Member code MemberCode
  //-
  //- MemberCode child.0 MemberContext
  //- MemberContext.post_child_text "."
  //- MemberContext.add_final_list_token true
  //-
  //- MemberContext child.0 MemberContextId
  //- MemberContextId.pre_text "A"
  //-
  //- MemberCode child.1 MemberName
  //- MemberName.pre_text "member"
  //- MemberCode child.2 MemberTy
  //- MemberTy.post_text "string"
  //- MemberCode child.3 MemberInit
  //- MemberInit.pre_text "'member'"
  member = 'member';

  //- @#0staticmember defines/binding StaticMember
  //- StaticMember code StaticMemberCode
  //-
  //- StaticMemberCode child.0 StaticMemberContext
  //- StaticMemberContext.post_child_text "."
  //- StaticMemberContext.add_final_list_token true
  //-
  //- StaticMemberContext child.0 StaticMemberContextId
  //- StaticMemberContextId.pre_text "A"
  //-
  //- StaticMemberCode child.1 StaticMemberName
  //- StaticMemberName.pre_text "staticmember"
  //- StaticMemberCode child.2 StaticMemberTy
  //- StaticMemberTy.post_text "string"
  //- StaticMemberCode child.3 StaticMemberInit
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
  //- BPropCode child.2 BPropInit
  //- BPropInit.pre_text "1"
  bprop: 1,
};

interface C {
  //- @cprop defines/binding CProp
  //- CProp code CPropCode
  //-
  //- CPropCode child.0 CPropContext
  //- CPropContext.post_child_text "."
  //- CPropContext.add_final_list_token true
  //-
  //- CPropContext child.0 CPropContextId
  //- CPropContextId.pre_text "C"
  //-
  //- CPropCode child.1 CPropName
  //- CPropName.pre_text "cprop"
  //- CPropCode child.2 CPropTy
  //- CPropTy.post_text "string"
  cprop: string;
}
