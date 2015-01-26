package pkg;

/** Tests Kythe inheritance-related nodes/edges. */
public class Inheritance {
  //- @A defines AClass
  static class A {
    //- @method1 defines AM1
    //- !{ @method1 defines AM1Overloaded }
    public void method1() {}

    //- @method1 defines AM1Overloaded
    //- !{ @method1 defines AM1 }
    public void method1(String overloaded) {}
  }

  //- @B defines BClass
  //- @A ref AClass
  //- BClass extends AClass
  //- !{ BClass implements AnyInterface }
  static class B extends A {
    @Override
    //- @method1 defines BM1
    //- BM1 overrides AM1
    //- !{ BM1 overrides AM1Overloaded
    //-    BM1 overrides/transitive AM1
    //-    BM1 overrides/transitive AM1Overloaded
    //-    BM1 overrides IM1 }
    public void method1() {}
  }

  //- @C defines CClass
  //- @B ref BClass
  //- CClass extends BClass
  //- CClass implements IInterface
  //- !{ CClass extends AClass
  //-    CCLass extends IInterface
  //-    CClass implements BClass }
  static class C extends B implements I {
    @Override
    //- @method1 defines CM1
    //- CM1 overrides IM1
    //- CM1 overrides BM1
    //- !{ CM1 overrides AM1
    //-    CM1 overrides/transitive IM1
    //-    CM1 overrides/transitive BM1 }
    //- CM1 overrides/transitive AM1
    public void method1() {}
  }

  //- @I defines IInterface
  static interface I {
    //- @method1 defines IM1
    public void method1();
  }
}
