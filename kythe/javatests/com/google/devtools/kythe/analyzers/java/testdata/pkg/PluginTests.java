package pkg;

//- @PluginTests defines/binding Class
public class PluginTests {

  @SpecialAnnotation
  //- @method defines/binding Method
  //- Method./extra/fact value
  //- SpecialMethod.node/kind function
  //- SpecialMethod.subkind special
  //- SpecialMethod /specializing/edge Method
  //- @method /special/defines/binding SpecialMethod
  public static void method() {}

  //- Class generates JVMClass
  //- Method generates JVMMethod
  //- JVMMethod /special/jvm/edge JVMClass

  /** A special annotation for special methods. */
  public @interface SpecialAnnotation {}
}
