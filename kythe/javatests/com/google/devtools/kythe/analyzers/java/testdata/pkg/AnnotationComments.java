package pkg;

// Tests for comments that should be ignored.
public class AnnotationComments {
  //- @+3fooString defines/binding FooString

  @SuppressWarnings("unchecked") // This does not documents fooString.
  public String fooString() { return ""; }
  //- !{ _UncheckedDoc documents FooString }
}
