// Verify that we support using pointers, references and pointer-to-members in
// non-type template parameters.

template <typename Type, Type Value>
struct Pair {
  using type = Type;
  static constexpr type value() { return Value; }
};

//- @Value defines/binding ValueType
struct Value {
  //- @member defines/binding ValueMember
  int member;
};

//- @fn defines/binding FnDecl
void fn();

//- @Alias defines/binding AliasDecl
//- @Value ref ValueType
//- @member ref ValueMember
using Alias = Pair<decltype(&Value::member),
  //- @Value ref ValueType
  //- @member ref ValueMember
  &Value::member>;


//- @FnPtrAlias defines/binding FnPtrAliasType
//- @fn ref FnDecl
using FnPtrAlias = Pair<decltype(&fn),
//- @fn ref FnDecl
      &fn>;

//- @FnRefAlias defines/binding FnRefAliasType
//- @fn ref FnDecl
using FnRefAlias = Pair<decltype(*fn),
//- @fn ref FnDecl
      fn>;
