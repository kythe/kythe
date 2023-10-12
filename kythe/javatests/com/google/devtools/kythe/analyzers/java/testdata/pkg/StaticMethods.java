package pkg;

@SuppressWarnings("unused")
public class StaticMethods {

  // - @member defines/binding PrivateMember
  private int member;

  // - @member defines/binding MemberFunc
  public static int member() {
    return 0;
  }

  // - @staticMember defines/binding PrivateStaticMember
  private static int staticMember;

  // - @staticMember defines/binding ProtectedStaticMemberFunc
  protected static int staticMember(int x) {
    return x;
  }

  // - @staticMember defines/binding PackageStaticMemberFunc
  static int staticMember(boolean b) {
    return b ? 1 : 0;
  }

  // - @staticMember defines/binding StaticMemberFunc
  public static int staticMember() {
    return 0;
  }

  // - @staticMethod defines/binding StaticBool
  public static void staticMethod(boolean b) {}

  // - @staticMethod defines/binding StaticInt
  public static void staticMethod(int b) {}
}

// - MemberFunc.tag/static _
// - ProtectedStaticMemberFunc.tag/static _
// - PackageStaticMemberFunc.tag/static _
// - StaticMemberFunc.tag/static _
// - StaticBool.tag/static _
// - StaticInt.tag/static _
