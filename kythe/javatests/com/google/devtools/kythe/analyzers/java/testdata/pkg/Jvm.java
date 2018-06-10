package pkg;

import java.util.Formattable;
import java.util.List;

//- @Jvm defines/binding ClassJava
//- ClassJava generates ClassJvm
public class Jvm {

  //- @intField defines/binding IntFieldJava
  //- IntFieldJava generates IntFieldJvm
  int intField;

  //- @Nested defines/binding NestedJava
  //- NestedJava generates NestedJvm
  public static class Nested {
    //- @Nested defines/binding NestedCtorJava
    //- NestedCtorJava generates NestedCtorJvm
    public Nested() {}
  }

  //- @Inner defines/binding InnerJava
  //- InnerJava generates InnerJvm
  public class Inner {
    //- @Inner defines/binding InnerCtorJava
    //- InnerCtorJava generates InnerCtorJvm
    public Inner() {}
  }

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
  public static class Generic<T extends Integer> {
    //- @tfield defines/binding TFieldJava
    //- TFieldJava.node/kind variable
    //- TFieldJava generates TFieldJvm
    private T tfield;

    //- @tmethod defines/binding TMethodJava
    //- TMethodJava.node/kind function
    //- TMethodJava generates TMethodJvm
    private void tmethod(T targ) {}

    //- @tlistmethod defines/binding TListMethodJava
    //- TListMethodJava.node/kind function
    //- TListMethodJava generates TListMethodJvm
    private void tlistmethod(List<T> targ) {}

    //- @tlistretmethod defines/binding TListRetMethodJava
    //- TListRetMethodJava.node/kind function
    //- TListRetMethodJava generates TListRetMethodJvm
    private List<T> tlistretmethod() {
      return null;
    }
  }

  //- @MultipleBoundGeneric defines/binding MBGenericAbs
  //- MBGenericAbs.node/kind abs
  //- MBGenericClass childof MBGenericAbs
  //- MBGenericClass.node/kind record
  //- MBGenericClass generates MBGenericJvm
  public static class MultipleBoundGeneric<T extends Integer & Formattable> {
    //- @mbtmethod defines/binding MBTMethodJava
    //- MBTMethodJava.node/kind function
    //- MBTMethodJava generates MBTMethodJvm
    private void mbtmethod(T targ) {}
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
