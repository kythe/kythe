// Checks whether function templates are given unique names.
// The well-formedness checks will fail on redundant or conflicting entries.
template <bool B, class T = void> struct enable_if {};
template <class T> struct enable_if<true, T> { using type = T; };

template <typename T1, typename T2> class same_type {
  static constexpr bool value = false;
};
template <typename T> class same_type<T, T> {
  static constexpr bool value = true;
};

template <typename T>
struct C {
  template <typename U=T>
  typename enable_if<same_type<U, int>::value, C&>::type operator=(C);

  template <typename U=T>
  typename enable_if<!same_type<U, int>::value, C&>::type operator=(C);
};

C<int> c_int;
C<float> c_float;
