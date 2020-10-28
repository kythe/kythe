template <typename T> class Included { };
template <typename T> void IF(T t) { }
template <typename T> T inc_var = 0;
#define INC_MACRO template <typename T> class IncludedMacro { };
