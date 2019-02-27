package pkg;

//- @+3AnnotationComments defines/binding AnnotationComments

@Deprecated // TODO(#3459): This should not annotate the class, but does.
public class AnnotationComments {
  //- { _ documents AnnotationComments }
  // The above test is broken and should be '!{ _ documents ...' instead

  //- @+3fooString defines/binding FooString

  @SuppressWarnings("unchecked") // This should not documents fooString.
  public String fooString() { return ""; }
  //- !{ _ documents FooString }
}
