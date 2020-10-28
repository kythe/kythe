package pkg;

public interface PrivateInterfaceMethods {
  public default void method() {
    //- @privateMethod ref PrivateMethod
    privateMethod();
  }

  //- @privateMethod defines/binding PrivateMethod
  private void privateMethod() {}

  public static void staticMethod() {
    //- @privateStaticMethod ref PrivateStaticMethod
    privateStaticMethod();
  }

  //- @privateStaticMethod defines/binding PrivateStaticMethod
  private static void privateStaticMethod() {}
}
