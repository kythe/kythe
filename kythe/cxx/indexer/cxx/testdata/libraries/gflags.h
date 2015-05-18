// gflags.h stripped of everything but {validator, flag} definitions.

#ifndef GFLAGS_GFLAGS_H_
#define GFLAGS_GFLAGS_H_

#include "gflags_declare.h"
namespace google {

#define GFLAGS_DLL_DECL  /* rewritten to be non-empty in windows dir */
#define GFLAGS_DLL_DEFINE_FLAG  /* rewritten to be non-empty in windows dir */

extern bool RegisterFlagValidator(const bool* flag,
                                  bool (*validate_fn)(const char*, bool));
extern bool RegisterFlagValidator(const int32* flag,
                                  bool (*validate_fn)(const char*, int32));
extern bool RegisterFlagValidator(const int64* flag,
                                  bool (*validate_fn)(const char*, int64));
extern bool RegisterFlagValidator(const uint64* flag,
                                  bool (*validate_fn)(const char*, uint64));
extern bool RegisterFlagValidator(const double* flag,
                                  bool (*validate_fn)(const char*, double));
extern bool RegisterFlagValidator(const std::string* flag,
                                  bool (*validate_fn)(const char*,
                                                      const std::string&));

#define DEFINE_validator(name, validator) \
    static const bool name##_validator_registered = \
            ::google::RegisterFlagValidator(&FLAGS_##name, validator)

class GFLAGS_DLL_DECL FlagRegisterer {
 public:
  FlagRegisterer(const char* name, const char* type,
                 const char* help, const char* filename,
                 void* current_storage, void* defvalue_storage);
};

}

#ifndef SWIG  // In swig, ignore the main flag declarations

#if defined(STRIP_FLAG_HELP) && STRIP_FLAG_HELP > 0
#define MAYBE_STRIPPED_HELP(txt) \
   (false ? (txt) : ::google::kStrippedFlagHelp)
#else
#define MAYBE_STRIPPED_HELP(txt) txt
#endif

// Each command-line flag has two variables associated with it: one
// with the current value, and one with the default value.  However,
// we have a third variable, which is where value is assigned; it's a
// constant.  This guarantees that FLAG_##value is initialized at
// static initialization time (e.g. before program-start) rather than
// than global construction time (which is after program-start but
// before main), at least when 'value' is a compile-time constant.  We
// use a small trick for the "default value" variable, and call it
// FLAGS_no<name>.  This serves the second purpose of assuring a
// compile error if someone tries to define a flag named no<name>
// which is illegal (--foo and --nofoo both affect the "foo" flag).
#define DEFINE_VARIABLE(type, shorttype, name, value, help)             \
  namespace fL##shorttype {                                             \
    static const type FLAGS_nono##name = value;                         \
    /* We always want to export defined variables, dll or no */         \
    GFLAGS_DLL_DEFINE_FLAG type FLAGS_##name = FLAGS_nono##name;        \
    type FLAGS_no##name = FLAGS_nono##name;                             \
    static ::google::FlagRegisterer o_##name( \
      #name, #type, MAYBE_STRIPPED_HELP(help), __FILE__,                \
      &FLAGS_##name, &FLAGS_no##name);                                  \
  }                                                                     \
  using fL##shorttype::FLAGS_##name

// For DEFINE_bool, we want to do the extra check that the passed-in
// value is actually a bool, and not a string or something that can be
// coerced to a bool.  These declarations (no definition needed!) will
// help us do that, and never evaluate From, which is important.
// We'll use 'sizeof(IsBool(val))' to distinguish. This code requires
// that the compiler have different sizes for bool & double. Since
// this is not guaranteed by the standard, we check it with a
// COMPILE_ASSERT.
namespace fLB {
struct CompileAssert {};
typedef CompileAssert expected_sizeof_double_neq_sizeof_bool[
                      (sizeof(double) != sizeof(bool)) ? 1 : -1];
template<typename From> double GFLAGS_DLL_DECL IsBoolFlag(const From& from);
GFLAGS_DLL_DECL bool IsBoolFlag(bool from);
}  // namespace fLB

#define DEFINE_bool(name, val, txt)                                     \
  namespace fLB {                                                       \
    typedef ::fLB::CompileAssert FLAG_##name##_value_is_not_a_bool[     \
            (sizeof(::fLB::IsBoolFlag(val)) != sizeof(double)) ? 1 : -1]; \
  }                                                                     \
  DEFINE_VARIABLE(bool, B, name, val, txt)

#define DEFINE_int32(name, val, txt) \
   DEFINE_VARIABLE(::google::int32, I, \
                   name, val, txt)

#define DEFINE_int64(name, val, txt) \
   DEFINE_VARIABLE(::google::int64, I64, \
                   name, val, txt)

#define DEFINE_uint64(name,val, txt) \
   DEFINE_VARIABLE(::google::uint64, U64, \
                   name, val, txt)

#define DEFINE_double(name, val, txt) \
   DEFINE_VARIABLE(double, D, name, val, txt)

// Strings are trickier, because they're not a POD, so we can't
// construct them at static-initialization time (instead they get
// constructed at global-constructor time, which is much later).  To
// try to avoid crashes in that case, we use a char buffer to store
// the string, which we can static-initialize, and then placement-new
// into it later.  It's not perfect, but the best we can do.

namespace fLS {

inline clstring* dont_pass0toDEFINE_string(char *stringspot,
                                           const char *value) {
  return new(stringspot) clstring();
}
inline clstring* dont_pass0toDEFINE_string(char *stringspot,
                                           const clstring &value) {
  return new(stringspot) clstring();
}
inline clstring* dont_pass0toDEFINE_string(char *stringspot,
                                           int value);
}  // namespace fLS

// We need to define a var named FLAGS_no##name so people don't define
// --string and --nostring.  And we need a temporary place to put val
// so we don't have to evaluate it twice.  Two great needs that go
// great together!
// The weird 'using' + 'extern' inside the fLS namespace is to work around
// an unknown compiler bug/issue with the gcc 4.2.1 on SUSE 10.  See
//    http://code.google.com/p/google-gflags/issues/detail?id=20
#define DEFINE_string(name, val, txt)                                       \
  namespace fLS {                                                           \
    using ::fLS::clstring;                                                  \
    static union { void* align; char s[sizeof(clstring)]; } s_##name[2];    \
    clstring* const FLAGS_no##name = ::fLS::                                \
                                   dont_pass0toDEFINE_string(s_##name[0].s, \
                                                             val);          \
    static ::google::FlagRegisterer o_##name(  \
        #name, "string", MAYBE_STRIPPED_HELP(txt), __FILE__,                \
        s_##name[0].s, new (s_##name[1].s) clstring(*FLAGS_no##name));      \
    extern GFLAGS_DLL_DEFINE_FLAG clstring& FLAGS_##name;                   \
    using fLS::FLAGS_##name;                                                \
    clstring& FLAGS_##name = *FLAGS_no##name;                               \
  }                                                                         \
  using fLS::FLAGS_##name

#endif

#endif

