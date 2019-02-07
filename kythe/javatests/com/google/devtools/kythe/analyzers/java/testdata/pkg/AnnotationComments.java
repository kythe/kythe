package pkg;

// Tests for comments that should be ignored.
public class AnnotationComments {
  //- @+3fooString defines/binding FooString

  @SuppressWarnings("unchecked") // TODO(#3459): This should not documents fooString, but does.
  public String fooString() { return ""; }
  // These tests are all busted:
  //- !{ UncheckedDoc2 documents FooString }
  //- UncheckedDoc2.node/kind doc
  //- UncheckedDoc2.text "TODO(#3459): This should not documents fooString, but does."
}
