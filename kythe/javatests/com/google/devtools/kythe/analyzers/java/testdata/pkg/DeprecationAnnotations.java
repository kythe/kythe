package pkg;

//- @+8DeprecationAnnotations defines/binding DeprecationClass

/**
 * This the javadoc summary fragment, not the {@code @deprecated} tag.
 *
 * @author this is an author tag, not the {@code @deprecated} tag.
 */
@Deprecated
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
}
