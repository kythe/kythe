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
  //- @intParam defines/binding Param0Java
  //- @objectParam defines/binding Param1Java
  //- Param0Java generates Param0Jvm
  //- Param1Java generates Param1Jvm
  public static void func(int intParam, Object objectParam) {}

  //- @Generic defines/binding GenericAbs
  //- GenericAbs.node/kind abs
  //- GenericClass childof GenericAbs
  //- GenericClass.node/kind record
  //- GenericClass generates GenericJvm
  public static class Generic<T> {
    //- @tfield defines/binding TFieldJava
    //- TFieldJava.node/kind variable
    //- TFieldJava generates TFieldJvm
    private T tfield;

    //- @tmethod defines/binding TMethodJava
    //- TMethodJava.node/kind function
    //- TMethodJava generates TMethodJvm
    private void tmethod(T targ) {}
  }

  //- @nope defines/binding NopeJava
  //- NopeJava generates NopeJvm
  public static <T> T nope() {
    return null;
  }

  // Ensure anonymous classes do not crash the JVM analyzer.
  static final Object OBJ = new Object() {};

  static void f() {
    // Ensure local classes do not crash the JVM analyzer.
    class LocalClass {}
  }

  //- @g defines/binding GJava
  //- GJava generates GJvm
  static void g(
      int[] arrayParam,
      boolean booleanParam,
      byte byteParam,
      char charParam,
      double doubleParam,
      float floatParam,
      int intParam,
      long longParam,
      short shortParam) {}

  //- @ints defines/binding VarArgsParamJava
  //- VarArgsParamJava generates VarArgsParamJvm
  static void varargs(int... ints) {}
}
