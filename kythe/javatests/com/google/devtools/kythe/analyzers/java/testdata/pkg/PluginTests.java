package pkg;

//- @PluginTests defines/binding Class
public class PluginTests {

  //- @PluginTests defines/binding Constructor
  PluginTests() {}

  @SpecialAnnotation
  //- @method defines/binding Method
  //- Method./extra/fact value
  //- SpecialMethod.node/kind function
  //- SpecialMethod.subkind special
  //- SpecialMethod /specializing/edge Method
  //- @method /special/defines/binding SpecialMethod
  public static void method() {}

  @SpecialAnnotation
  //- @field defines/binding Field
  public int field;

  //- Class generates JVMClass
  //- Method generates JVMMethod
  //- Field generates JVMField
  //- JVMMethod /special/jvm/edge JVMClass
  //- JVMField /special/jvm/edge JVMClass
  //- Constructor generates _JVMConstructor
  // TODO(#2993): re-enable this assertion and preceding reference.
  // JVMConstructor /special/jvm/edge JVMClass

  /** A special annotation for special methods. */
  public @interface SpecialAnnotation {}
}
