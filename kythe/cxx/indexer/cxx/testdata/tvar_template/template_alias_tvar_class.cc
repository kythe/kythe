template <typename T2>
class C {
 public:
  template <typename T>
   //- @S defines/binding SDef
  struct S {};
};

void g() {
  //- @S ref SDef
  C<int>::S<int> s;
}
