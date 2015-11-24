// Check support for anonymous unions.

union U {
  union {
    //- @field defines/binding MemberOfAnonymousUnion
    int field;
  };
  //- @field ref MemberOfAnonymousUnion
  static_assert(&U::field != nullptr, "");
};
