package pkg;

/** Tests Kythe inheritance-related nodes/edges. */
@SuppressWarnings("MissingOverride")
public class Inheritance {
  // - @obj defines/binding V
  // - V typed Obj
  Object obj;

  // - @A defines/binding AClass
  // - AClass extends Obj
  static class A {
    // - @method1 defines/binding AM1
    // - !{ @method1 defines/binding AM1Overloaded }
    public void method1() {}

    // - @method1 defines/binding AM1Overloaded
    // - !{ @method1 defines/binding AM1 }
    public void method1(String overloaded) {}

    // - @method2 defines/binding AM2
    public void method2() {}
  }

  // - @B defines/binding BClass
  // - @A ref AClass
  // - BClass extends AClass
  // - !{ AnyInterface.node/kind interface
  // -    BClass extends AnyInterface }
  static class B extends A {
    @Override
    // - @method1 defines/binding BM1
    // - BM1 overrides AM1
    // - !{ BM1 overrides AM1Overloaded
    // -    BM1 overrides/transitive AM1
    // -    BM1 overrides/transitive AM1Overloaded
    // -    BM1 overrides IM1 }
    public void method1() {}
  }

  // - @C defines/binding CClass
  // - @B ref BClass
  // - CClass extends BClass
  // - CClass extends IInterface
  // - !{ CClass extends AClass
  // -    CClass extends IInterface
  // -    CClass extends BClass }
  static class C extends B implements I {
    @Override
    // - @method1 defines/binding CM1
    // - CM1 overrides IM1
    // - CM1 overrides BM1
    // - !{ CM1 overrides AM1
    // -    CM1 overrides/transitive IM1
    // -    CM1 overrides/transitive BM1 }
    // - CM1 overrides/transitive AM1
    public void method1() {}

    // There is no BM2 so CM2 directly overrides AM2.
    // - @method2 defines/binding CM2
    // - CM2 overrides AM2
    // - !{ CM2 overrides/transitive AM2 }
    // - CM2 overrides IM2
    public void method2() {}
  }

  // - @I defines/binding IInterface
  static interface I {
    // - @method1 defines/binding IM1
    public void method1();

    // - @method2 defines/binding IM2
    public void method2();
  }

  // - @enumBase defines/binding EN
  // - EN typed EnumBase
  Enum<E> enumBase;

  // - @E defines/binding EEnum
  // - EEnum extends EnumBase
  private static enum E {}

  static interface DTop {
    // - @m defines/binding DTopM
    public void m();
  }

  static interface DMid extends DTop {
    // - @m defines/binding DMidM
    // - DMidM overrides DTopM
    // - !{ DMidM overrides/transitive DTopM }
    public void m();
  }

  static class DBot implements DMid, DTop {
    // - @m defines/binding DBotM
    // - DBotM overrides DMidM
    // - DBotM overrides DTopM
    // - !{ DBotM overrides/transitive DTopM }
    // - !{ DBotM overrides/transitive DMidM }
    public void m() {}
  }
}
