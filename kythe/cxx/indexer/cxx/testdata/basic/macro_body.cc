// A fairly involved and convoluted macro that, through several layers of
// expansion, results in a variety of ranges both from the macro body and
// macro arguments.  We want to ensure that only ranges originating from
// explicit macro arguments are expanded and all others are zero-width from
// the front of the macro expansion.
//- @ASSIGN_OR_RETURN defines/binding TerribleMacro
#define ASSIGN_OR_RETURN(...)                                            \
  IMPL_GET_VARIADIC_(                                                    \
      (__VA_ARGS__, IMPL_ASSIGN_OR_RETURN_3_, IMPL_ASSIGN_OR_RETURN_2_)) \
  (__VA_ARGS__)
#define IMPL_GET_VARIADIC_HELPER_(_1, _2, _3, NAME, ...) NAME
#define IMPL_GET_VARIADIC_(args) IMPL_GET_VARIADIC_HELPER_ args
#define IMPL_ASSIGN_OR_RETURN_2_(lhs, rexpr) \
  IMPL_ASSIGN_OR_RETURN_3_(lhs, rexpr, _)
#define IMPL_ASSIGN_OR_RETURN_3_(lhs, rexpr, error)                            \
  IMPL_ASSIGN_OR_RETURN_(IMPL_CONCAT_(_status_or_value, __LINE__), lhs, rexpr, \
                         error)
#define IMPL_CONCAT_INNER_(x, y) x##y
#define IMPL_CONCAT_(x, y) IMPL_CONCAT_INNER_(x, y)
#define IMPL_ASSIGN_OR_RETURN_(statusor, lhs, rexpr, error) \
  auto statusor = (rexpr);                                  \
  if (!statusor) {                                          \
    int* _ = nullptr;                                       \
    return (error);                                         \
  }                                                         \
  lhs = *statusor;

//- @get defines/binding GetDecl
int* get(int, int);
int* f() {
  //- @a defines/binding AVarDecl
  //- @b defines/binding BVarDecl
  int a, b;

  //- @ASSIGN_OR_RETURN ref/expands TerribleMacro
  //- @r defines/binding RVarDecl
  //- @int ref IntBuiltin
  //- RVarDecl typed IntBuiltin
  //- @get ref GetDecl
  //- @"get(a, b)" ref/call GetDecl
  //- @a ref AVarDecl
  //- @b ref BVarDecl
  //- // The full macro expansion should not define anything.
  //- !{ @"ASSIGN_OR_RETURN(int r, get(a, b))" defines/binding _ }
  ASSIGN_OR_RETURN(int r, get(a, b));
}
