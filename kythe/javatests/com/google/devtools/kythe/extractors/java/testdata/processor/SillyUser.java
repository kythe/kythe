package processor;

/**
 * A silly class that uses the @Silly annotation and expects SillyGenerated to be generated by the
 * annotation processor.
 */
@Silly
public class SillyUser {
  boolean x = new SillyGenerated().isSilly();
}
