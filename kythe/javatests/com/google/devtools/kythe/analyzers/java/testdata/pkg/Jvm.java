package pkg;

import java.util.Formattable;
import java.util.List;

@SuppressWarnings({"unused", "ClassCanBeStatic"})
// - @Jvm defines/binding ClassJava
// - ClassJava generates ClassJvm
// - ClassJava named ClassJvm
public class Jvm {

  // - @intField defines/binding IntFieldJava
  // - IntFieldJava generates IntFieldJvm
  // - IntFieldJava named IntFieldJvm
  int intField;

  // - @Nested defines/binding NestedJava
  // - NestedJava generates NestedJvm
  // - NestedJava named NestedJvm
  public static class Nested {
    // - @Nested defines/binding NestedCtorJava
    // - NestedCtorJava generates NestedCtorJvm
    // - NestedCtorJava named NestedCtorJvm
    public Nested() {}
  }

  // - @Inner defines/binding InnerJava
  // - InnerJava generates InnerJvm
  // - InnerJava named InnerJvm
  public class Inner {
    // - @Inner defines/binding InnerCtorJava
    // - InnerCtorJava generates InnerCtorJvm
    // - InnerCtorJava named InnerCtorJvm
    public Inner() {}

    // - @InnerInner defines/binding InnerInnerJava
    // - InnerInnerJava generates InnerInnerJvm
    // - InnerInnerJava named InnerInnerJvm
    public class InnerInner {
      // - @InnerInner defines/binding InnerInnerCtorJava
      // - InnerInnerCtorJava generates InnerInnerCtorJvm
      // - InnerInnerCtorJava named InnerInnerCtorJvm
      public InnerInner() {}
    }
  }

  // - @methodWithInnerParam defines/binding MethodWithInnerParamJava
  // - MethodWithInnerParamJava.node/kind function
  // - MethodWithInnerParamJava generates MethodWithInnerParamJvm
  // - MethodWithInnerParamJava named MethodWithInnerParamJvm
  private void methodWithInnerParam(Inner inner) {}

  // - @func defines/binding FuncJava
  // - FuncJava generates FuncJvm
  // - FuncJava named FuncJvm
  // - @intParam defines/binding Param0Java
  // - @objectParam defines/binding Param1Java
  // - Param0Java generates Param0Jvm
  // - Param1Java generates Param1Jvm
  // - Param0Java named Param0Jvm
  // - Param1Java named Param1Jvm
  public static void func(int intParam, Object objectParam) {}

  // - @Generic defines/binding GenericClass
  // - GenericClass.node/kind record
  // - GenericClass generates GenericJvm
  // - GenericClass named GenericJvm
  public static class Generic<T extends Integer> {
    // - @tfield defines/binding TFieldJava
    // - TFieldJava.node/kind variable
    // - TFieldJava generates TFieldJvm
    // - TFieldJava named TFieldJvm
    private T tfield;

    // - @tmethod defines/binding TMethodJava
    // - TMethodJava.node/kind function
    // - TMethodJava generates TMethodJvm
    // - TMethodJava named TMethodJvm
    private void tmethod(T targ) {}

    // - @tlistmethod defines/binding TListMethodJava
    // - TListMethodJava.node/kind function
    // - TListMethodJava generates TListMethodJvm
    // - TListMethodJava named TListMethodJvm
    private void tlistmethod(List<T> targ) {}

    // - @tlistretmethod defines/binding TListRetMethodJava
    // - TListRetMethodJava.node/kind function
    // - TListRetMethodJava generates TListRetMethodJvm
    // - TListRetMethodJava named TListRetMethodJvm
    private List<T> tlistretmethod() {
      return null;
    }
  }

  // - @MultipleBoundGeneric defines/binding MBGenericClass
  // - MBGenericClass.node/kind record
  // - MBGenericClass generates MBGenericJvm
  // - MBGenericClass named MBGenericJvm
  public static class MultipleBoundGeneric<T extends Integer & Formattable> {
    // - @mbtmethod defines/binding MBTMethodJava
    // - MBTMethodJava.node/kind function
    // - MBTMethodJava generates MBTMethodJvm
    // - MBTMethodJava named MBTMethodJvm
    private void mbtmethod(T targ) {}
  }

  @SuppressWarnings("TypeParameterUnusedInFormals")
  // - @nope defines/binding NopeJava
  // - NopeJava generates NopeJvm
  // - NopeJava named NopeJvm
  public static <T> T nope() {
    return null;
  }

  // Ensure anonymous classes do not crash the JVM analyzer.
  static final Object OBJ = new Object() {};

  static void f() {
    // Ensure local classes do not crash the JVM analyzer.
    class LocalClass {}
  }

  // - @g defines/binding GJava
  // - GJava generates GJvm
  // - GJava named GJvm
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

  // - @ints defines/binding VarArgsParamJava
  // - VarArgsParamJava generates VarArgsParamJvm
  // - VarArgsParamJava named VarArgsParamJvm
  static void varargs(int... ints) {}

  // - @E defines/binding Enum
  // - Enum named vname("pkg.Jvm$E",_,_,_,"jvm")
  public static enum E {
    // - @UNKNOWN defines/binding EnumValue
    // - EnumValue named vname("pkg.Jvm$E.UNKNOWN",_,_,_,"jvm")
    UNKNOWN;
  }
}
