package pkg;

//- @Jvm defines/binding ClassJava
//- ClassJava generates ClassJvm
public class Jvm {

  //- @intField defines/binding IntFieldJava
  //- IntFieldJava generates IntFieldJvm
  int intField;

  //- @Inner defines/binding InnerJava
  //- InnerJava generates InnerJvm
  public static class Inner {}

  //- @func defines/binding FuncJava
  //- FuncJava generates FuncJvm
  public static void func(int i, Object o) {}

  //- @nope defines/binding NopeJava
  //- NopeJava generates NopeJvm
  public static <T> T nope() {
    return null;
  }
}
