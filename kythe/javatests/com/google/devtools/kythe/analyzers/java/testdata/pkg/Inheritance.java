package pkg;

/** Tests Kythe inheritance-related nodes/edges. */
public class Inheritance {
  //- @obj defines/binding V
  //- V typed Obj
  Object obj;

  //- @A defines/binding AClass
  //- AClass extends Obj
  static class A {
    //- @method1 defines/binding AM1
    //- !{ @method1 defines/binding AM1Overloaded }
    public void method1() {}

    //- @method1 defines/binding AM1Overloaded
    //- !{ @method1 defines/binding AM1 }
    public void method1(String overloaded) {}
  }

  //- @B defines/binding BClass
  //- @A ref AClass
  //- BClass extends AClass
  //- !{ AnyInterface.node/kind interface
  //-    BClass extends AnyInterface }
  static class B extends A {
    @Override
    //- @method1 defines/binding BM1
    //- BM1 overrides AM1
    //- !{ BM1 overrides AM1Overloaded
    //-    BM1 overrides/transitive AM1
    //-    BM1 overrides/transitive AM1Overloaded
    //-    BM1 overrides IM1 }
    public void method1() {}
  }

  //- @C defines/binding CClass
  //- @B ref BClass
  //- CClass extends BClass
  //- CClass extends IInterface
  //- !{ CClass extends AClass
  //-    CCLass extends IInterface
  //-    CClass extends BClass }
  static class C extends B implements I {
    @Override
    //- @method1 defines/binding CM1
    //- CM1 overrides IM1
    //- CM1 overrides BM1
    //- !{ CM1 overrides AM1
    //-    CM1 overrides/transitive IM1
    //-    CM1 overrides/transitive BM1 }
    //- CM1 overrides/transitive AM1
    public void method1() {}
  }

  //- @I defines/binding IInterface
  static interface I {
    //- @method1 defines/binding IM1
    public void method1();
  }
}
