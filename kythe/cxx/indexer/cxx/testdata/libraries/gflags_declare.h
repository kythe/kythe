// gflags_declare.h stripped of everything but flag declarations.

#ifndef GFLAGS_DECLARE_H_
#define GFLAGS_DECLARE_H_

namespace std {
struct string {  };
}

namespace google {
typedef int int32;
typedef unsigned uint32;
typedef long int64;
typedef unsigned long uint64;
}


#define GFLAGS_DLL_DECLARE_FLAG  /* rewritten to be non-empty in windows dir */
void *operator new(unsigned long, void*) throw();
namespace fLS {

typedef std::string clstring;

}

#define DECLARE_VARIABLE(type, shorttype, name) \
  /* We always want to import declared variables, dll or no */ \
  namespace fL##shorttype { extern GFLAGS_DLL_DECLARE_FLAG type FLAGS_##name; } \
  using fL##shorttype::FLAGS_##name

#define DECLARE_bool(name) \
  DECLARE_VARIABLE(bool, B, name)

#define DECLARE_int32(name) \
  DECLARE_VARIABLE(::google::int32, I, name)

#define DECLARE_int64(name) \
  DECLARE_VARIABLE(::google::int64, I64, name)

#define DECLARE_uint64(name) \
  DECLARE_VARIABLE(::google::uint64, U64, name)

#define DECLARE_double(name) \
  DECLARE_VARIABLE(double, D, name)

#define DECLARE_string(name) \
  namespace fLS {                       \
  using ::fLS::clstring;                \
  extern GFLAGS_DLL_DECLARE_FLAG ::fLS::clstring& FLAGS_##name; \
  }                                     \
  using fLS::FLAGS_##name

#endif

