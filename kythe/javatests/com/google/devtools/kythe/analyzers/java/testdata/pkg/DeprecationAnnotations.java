package pkg;

//- @+9DeprecationAnnotations defines/binding DeprecationClass

/**
 * This the javadoc summary fragment, not the {@code @deprecated} tag.
 *
 * @author this is an author tag, not the {@code @deprecated} tag.
 */
@Deprecated
@SuppressWarnings("ClassCanBeStatic")
public class DeprecationAnnotations {
  //- DeprecationClass.tag/deprecated ""

  //- @+3deprecatedMethod defines/binding DeprecationMethod

  @Deprecated
  void deprecatedMethod() {}
  //- DeprecationMethod.tag/deprecated ""

  //- @+3deprecatedField defines/binding DeprecationField

  @Deprecated
  int deprecatedField;
  //- DeprecationField.tag/deprecated ""

  //- InnerClass.node/kind record
  //- InnerClass.tag/deprecated ""
  //- @+3Inner defines/binding InnerClass

  @Deprecated
  public class Inner<E> {}

  //- Method.node/kind function
  //- Method.tag/deprecated ""
  //- @+3method defines/binding Method

  @Deprecated
  <T> void method(T t) {}
}
