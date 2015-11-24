// Check support for GNU's "anonymous struct" extension.
//
// This is very similar to standard C++'s anonymous unions, and Clang's AST
// models them in the same way with IndirectFieldDecls in parent scopes.
//
// (Amusingly?) this testcase crashes GCC 4.8.

union U {
  struct {
    //- @field defines/binding StructField
    int field;
  };
  //- @field ref StructField
  static_assert(&U::field != nullptr, "");
};
