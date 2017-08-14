package pkg;

public class PluginTests {

  @SpecialAnnotation
  //- @method defines/binding Method
  //- Method./extra/fact value
  //- SpecialMethod.node/kind function
  //- SpecialMethod.subkind special
  //- SpecialMethod /specializing/edge Method
  //- @method /special/defines/binding SpecialMethod
  public static void method() {}

  /** A special annotation for special methods. */
  public @interface SpecialAnnotation {}
}
