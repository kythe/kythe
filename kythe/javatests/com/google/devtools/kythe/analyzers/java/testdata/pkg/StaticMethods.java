package pkg;

public class StaticMethods {

  //- @member defines/binding PrivateMember
  private int member;

  //- @member defines/binding MemberFunc
  public static int member() { return 0; }

  //- @staticMethod defines/binding StaticBool
  public static void staticMethod(boolean b) {}

  //- @staticMethod defines/binding StaticInt
  public static void staticMethod(int b) {}
}
