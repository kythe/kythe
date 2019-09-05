namespace absl {
template <typename T>
class unique_ptr {
 public:
  explicit unique_ptr(T* data) : data_(data) {}
  unique_ptr(unique_ptr&& other) : data_(other.data_) { other.data_ = nullptr; }

 private:
  T* data_;
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(args...));
}
}  // namespace absl

//- @B defines/binding StructB
struct B {
  //- @B defines/binding ConsB1
  explicit B(int, void*);
  //- @B defines/binding ConsB2
  explicit B(char);
};

void f() {
  using ::absl::make_unique;

  //- @B ref StructB
  //- @"make_unique<B>" ref ConsB1
  //- @"::absl::make_unique<B>(1, nullptr)" ref/call ConsB1
  ::absl::make_unique<B>(1, nullptr);

  //- @B ref StructB
  //- @"make_unique<B>" ref ConsB2
  //- @"make_unique<B>('a')" ref/call ConsB2
  make_unique<B>('a');
}


namespace std {
inline namespace v1 {
template <typename T, typename... Args>
absl::unique_ptr<T> make_unique(Args&&... args) {
  return absl::unique_ptr<T>(new T(args...));
}
}
}

void g() {
  //- @B ref StructB
  //- @"make_unique<B>" ref ConsB1
  //- @"::std::make_unique<B>(1, nullptr)" ref/call ConsB1
  ::std::make_unique<B>(1, nullptr);
}
