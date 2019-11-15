// Tests that expression rvalues do not name a new scope a VName signature.

const a = class A {
  //- @foo defines/binding vname("a.foo", _, _, _, _)
  foo = 0;
};

class BB {
  //- @cc defines/binding vname("BB.cc", _, _, _, _)
  static cc = class C {
    //- @d defines/binding vname("BB.cc.d", _, _, _, _)
    d: number;
  };
  static ee = {
    //- @f defines/binding vname("BB.ee.f", _, _, _, _)
    f: 0
  };
}
