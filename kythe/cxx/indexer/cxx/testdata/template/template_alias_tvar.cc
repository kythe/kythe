template <typename T2>
class C {
 public:
  template <typename T>
   //- @f defines/binding FDef
  const T f() const { return {}; }
};

void g(C<int>& c) {
  //- @f ref FRef
  //- FRef param.0 FDef
  c.f<int>();
}
