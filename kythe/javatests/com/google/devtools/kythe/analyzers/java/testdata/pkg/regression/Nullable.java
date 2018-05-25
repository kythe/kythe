package pkg.regression;

import java.lang.annotation.ElementType;
import java.lang.annotation.Target;

@Target({ElementType.TYPE_USE})
public @interface Nullable {}

/**
 * Ensure that {@link ElementType.TYPE_USE} annotations do not cause the Java indexer to run into an
 * infinite loop while generating type variable's signature.
 */
class TestNullable {
  // - @genericT defines/binding GenericT
  static <T> void genericT(@Nullable T x) {}
}
