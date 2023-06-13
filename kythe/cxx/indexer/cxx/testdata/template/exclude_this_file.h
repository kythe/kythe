template <typename T> class Excluded { };
template <typename T> void EF(T t) { }
template <typename T> T exc_var = 0;
#define EXC_MACRO template <typename T> class ExcludedMacro { };
