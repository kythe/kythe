// With aliasing on, we don't record childof edges, even under
// multiple layers of implicit template expansion.
template <template <typename T> class Tmpl>
struct TemplateSel { };

template <template <typename T> class Tv1>
struct Templates1 {
  typedef TemplateSel<Tv1> Box;
};

template <class TestSel>
//- @T1 defines/binding RecT1
//- RecT1.node/kind record
class T1 {
 public:
//- @f defines/binding FnF
//- FnF childof RecT1
  static bool f() {
    return true;
  }
};

template <typename T2V>
class T2 {
 public:
  static bool g() {
    T1<typename T2V::Box>::f();
    return true;
  }
};

template <typename T> struct Content { };
bool b = T2<Templates1<Content>>::g();
