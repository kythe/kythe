// Tests that expression rvalues do not name a new scope a VName signature.

const a = class A {
  //- @foo defines/binding vname("testdata/rvalue/a.foo", _, _, _, _)
  foo = 0;
};

class BB {
  //- @cc defines/binding vname("testdata/rvalue/BB.cc", _, _, _, _)
  static cc = class C {
    //- @d defines/binding vname("testdata/rvalue/BB.cc.d", _, _, _, _)
    d: number;
  };
  static ee = {
    //- @f defines/binding vname("testdata/rvalue/BB.ee.f", _, _, _, _)
    f: 0
  };
}
