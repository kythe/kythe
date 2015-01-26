// This test checks (via well-formedness) that we emit distinct names for
// operator== and operator!= below.
namespace std {

template <typename T>
class allocator {
 public:
  allocator() throw() {}
};

template <typename T1, typename T2>
inline bool operator==(const allocator<T1>&, const allocator<T2>&) {
  return true;
}

template <typename T1, typename T2>
inline bool operator!=(const allocator<T1>&, const allocator<T2>&) {
  return false;
}

}  // namespace std
