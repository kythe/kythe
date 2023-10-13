package pkg;

// - @PluginTests defines/binding Class
public class PluginTests {

  @SpecialAnnotation
  // - @PluginTests defines/binding Constructor
  PluginTests() {}

  @SpecialAnnotation
  // - @method defines/binding Method
  // - Method./extra/fact value
  // - SpecialMethod.node/kind function
  // - SpecialMethod.subkind special
  // - SpecialMethod /specializing/edge Method
  // - @method /special/defines/binding SpecialMethod
  public static void method() {}

  @SpecialAnnotation
  // - @field defines/binding Field
  public int field;

  // - Class generates JVMClass
  // - Method generates JVMMethod
  // - Field generates JVMField
  // - Constructor generates JVMConstructor
  // - JVMMethod /special/jvm/edge JVMClass
  // - JVMField /special/jvm/edge JVMClass
  // - JVMConstructor /special/jvm/edge JVMClass

  /** A special annotation for special methods. */
  public @interface SpecialAnnotation {}
}
