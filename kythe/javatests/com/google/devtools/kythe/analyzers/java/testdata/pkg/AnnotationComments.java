package pkg;

//- @+4AnnotationComments defines/binding AnnotationComments

// This isn't documentation.
@Deprecated // This does not document AnnotationComments
public class AnnotationComments {
  //- !{ _ documents AnnotationComments }

  //- @+3fooString defines/binding FooString

  @SuppressWarnings("unchecked") // This should not documents fooString.
  public String fooString() { return ""; }
  //- !{ _ documents FooString }
}
