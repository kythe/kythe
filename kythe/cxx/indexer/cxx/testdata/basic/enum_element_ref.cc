//- @S defines/binding StructS
struct S {
  //- @EC defines/binding EnumClass
  //- @M defines/binding EnumClassMem
  enum class EC { M };
  //- @M defines/binding LegacyEnumMem
  enum E { M };
};

//- @S ref StructS
//- @EC ref EnumClass
//- @M ref EnumClassMem
auto i = S::EC::M;

//- @S ref StructS
//- @M ref LegacyEnumMem
auto j = S::M;
