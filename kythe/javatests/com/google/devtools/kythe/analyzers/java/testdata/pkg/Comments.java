package pkg;

//- @+11"java.lang.Integer" ref/doc IntegerClass
//- @+10String ref/doc StringClass
//- @+10Inner ref/doc InnerClass

//- DocComment.node/kind anchor
//- DocComment documents CommentsClass
//- DocComment.loc/start @^+4"/**"
//- DocComment.loc/end @$+6"*/"
//- @+6Comments defines/binding CommentsClass

/**
 * This is a Javadoc comment with links to {@link String}, {@link java.lang.Integer}, and
 * {@link Inner}.
 */
public class Comments
    implements Comparable<Comments> {

  //- DocNode.node/kind doc
  //- DocNode documents CommentsClass
  //- DocNode param.0 StringClass
  //- DocNode param.1 IntegerClass
  //- DocNode param.2 InnerClass
  //- DocNode.text " This is a Javadoc comment with links to {@link [String]}, {@link [java.lang.Integer]}, and\n {@link [Inner]}.\n"

  //- @+3"// inline comment here" documents FieldOne
  //- @+2fieldOne defines/binding FieldOne

  private static int fieldOne; // inline comment here

  //- @+3"// fieldTwo represents the universe" documents FieldTwo
  //- @+3fieldTwo defines/binding FieldTwo

  // fieldTwo represents the universe
  private static String fieldTwo;

  //- @+3"// This comments the Inner class." documents InnerClass
  //- @+3Inner defines/binding InnerClass

  // This comments the Inner class.
  public static class Inner {}

  //- @+5"// a second, weirdly-placed comment" documents InnerI
  //- @+5"// this also comments the interface" documents InnerI
  //- @+3"/* This comments the InnerI interface. */" documents InnerI
  //- @+3InnerI defines/binding InnerI

  /* This comments the InnerI interface. */ // a second, weirdly-placed comment
  static interface InnerI {} // this also comments the interface

  //- @+5InnerE ref/doc InnerE

  //- @+3"/** This comments the {@link InnerE} enum. */" documents InnerE
  //- @+3InnerE defines/binding InnerE

  /** This comments the {@link InnerE} enum. */
  static enum InnerE {

    //- InnerEDoc documents InnerE
    //- InnerEDoc.node/kind doc
    //- InnerEDoc param.0 InnerE
    //- InnerEDoc.text "This comments the {@link [InnerE]} enum. "

    //- @+3"// This comments SOME_VALUE." documents SomeValue
    //- @+3SOME_VALUE defines SomeValue

    // This comments SOME_VALUE.
    SOME_VALUE,

    //- @+3"/** This documents {@link ANOTHER_VALUE}. */" documents AnotherValue
    //- @+3ANOTHER_VALUE defines AnotherValue

    /** This documents {@link ANOTHER_VALUE}. */
    ANOTHER_VALUE;
  }

  //- @+3"/** This documents {@link #toString()}. */" documents ToString
  //- @+3toString defines/binding ToString

  /** This documents {@link #toString()}. */
  @Override public String toString() { return "null"; }

  //- @+3"/** This documents {@link #compareTo()} over {@link Override}. */" documents CompareTo
  //- @+4compareTo defines/binding CompareTo

  /** This documents {@link #compareTo()} over {@link Override}. */
  @Override
  public int compareTo(Comments o) { return 0; }
  //- OverrideDoc documents CompareTo
  //- OverrideDoc.node/kind doc
  //- OverrideDoc param.0 Override
  //- OverrideDoc.text "This documents {@link #compareTo()} over {@link [Override]}. "
}
