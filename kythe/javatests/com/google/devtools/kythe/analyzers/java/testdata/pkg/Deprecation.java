package pkg;

//- @+9Deprecation defines/binding DeprecationClass

/**
 * This the javadoc summary fragment, not the {@code @deprecated} tag.
 *
 * @deprecated this class is obsolete; prefer {@link Comments}
 * @author this is an author tag, not the {@code @deprecated} tag.
 */
@Deprecated
public class Deprecation {
  //- DeprecationClass.tag/deprecated "this class is obsolete; prefer {@link Comments}"

  //- @+4deprecatedMethod defines/binding DeprecationMethod

  /** @deprecated this method is obsolete */
  @Deprecated
  void deprecatedMethod() {}
  //- DeprecationMethod.tag/deprecated "this method is obsolete"

  //- @+4deprecatedField defines/binding DeprecationField

  /** @deprecated this field is obsolete */
  @Deprecated
  int deprecatedField;
  //- DeprecationField.tag/deprecated "this field is obsolete"

  //- @+3deprecatedFieldEmpty defines/binding DeprecationFieldEmpty

  /** @deprecated */
  int deprecatedFieldEmpty;
  //- DeprecationFieldEmpty.tag/deprecated ""
}
