template <typename T2>
class C {
 public:
  template <typename T>
   //- @A defines/binding ADef
   using A = T;
};

void g() {
  //- @A ref ADef
  C<int>::A<int> x;
}
