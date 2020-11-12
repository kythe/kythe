// - @pkg ref Package
package pkg;

// - @+8"java.lang.Integer" ref/doc IntegerClass
// - @+7String ref/doc StringClass
// - @+7Inner ref/doc InnerClass
// - @+6pkg ref/doc Package
// - @+7Comments defines/binding CommentsClass

@SuppressWarnings("unused")
/**
 * This is a Javadoc comment with links to {@link String}, {@link java.lang.Integer}, {@link Inner},
 * and {@link pkg}. It references {@value IM_A_LONG}.
 */
public class Comments implements Comparable<Comments> {

  // - @long ref/doc LongBuiltin
  // - LongBuiltin.node/kind tbuiltin
  /** This is a {@link long} with a value of {@value}. */
  public static final long IM_A_LONG = 4;

  public static final int BLAH80 = 4;

  // - DocNode.node/kind doc
  // - DocNode.subkind "javadoc"
  // - DocNode documents CommentsClass
  // - DocNode param.0 StringClass
  // - DocNode param.1 IntegerClass
  // - DocNode param.2 InnerClass
  // - DocNode param.3 Package
  // - DocNode.text " This is a Javadoc comment with links to {@link [String]}, {@link
  // [java.lang.Integer]},\n {@link [Inner]}, and {@link [pkg]}. It references {@value
  // [IM_A_LONG]}.\n"

  // - @fieldOne defines/binding _FieldOne
  private static int fieldOne; // inline comment here

  // - FieldTwoDoc documents FieldTwo
  // - FieldTwoDoc.node/kind doc
  // - !{ FieldTwoDoc.subkind _AnySubkind }
  // - FieldTwoDoc.text "fieldTwo represents the universe"
  // - @+3fieldTwo defines/binding FieldTwo

  // fieldTwo represents the universe
  private static String fieldTwo;

  // - @+3fieldThree defines/binding FieldThree
  // - @+3fieldFour defines/binding FieldFour

  private static int fieldThree; // EOL comment
  private static int fieldFour;

  // - FieldThreeDoc documents FieldThree
  // - FieldThreeDoc.node/kind doc
  // - FieldThreeDoc.text "EOL comment"
  // - !{ _FieldFourDoc documents FieldFour }

  // - @+3fieldFive defines/binding FieldFive
  // - @+2fieldSix defines/binding FieldSix

  private static int fieldFive, fieldSix; // EOL comment both

  // - FieldBothInlineDoc documents FieldFive
  // - FieldBothInlineDoc documents FieldSix
  // - FieldBothInlineDoc.text "EOL comment both"

  // - @+4fieldSeven defines/binding FieldSeven
  // - @+3fieldEight defines/binding FieldEight

  // above comment both
  private static int fieldSeven, fieldEight;

  // - FieldBothAboveDoc documents FieldSeven
  // - FieldBothAboveDoc documents FieldEight
  // - FieldBothAboveDoc.text "above comment both"

  // - InnerDoc documents InnerClass
  // - InnerDoc.node/kind doc
  // - InnerDoc.text "This comments the Inner class."
  // - @+3Inner defines/binding InnerClass

  // This comments the Inner class.
  public static class Inner {}

  // - InnerIDoc documents InnerI
  // - InnerIDoc.node/kind doc
  // - InnerIDoc.text "This comments the InnerI interface."
  // - InnerIWeirdDoc documents InnerI
  // - InnerIWeirdDoc.node/kind doc
  // - InnerIWeirdDoc.text "a second, weirdly-placed comment"
  // - @+3InnerI defines/binding InnerI

  /* This comments the InnerI interface. */
  // a second, weirdly-placed comment
  static interface InnerI {} // this also comments the interface

  // - @+3InnerE ref/doc InnerE
  // - @+3InnerE defines/binding InnerE

  /** This comments the {@link InnerE} enum. */
  static enum InnerE {

    // - InnerEDoc documents InnerE
    // - InnerEDoc.node/kind doc
    // - InnerEDoc param.0 InnerE
    // - InnerEDoc.text "This comments the {@link [InnerE]} enum. "

    // - SomeValDoc documents SomeValue
    // - SomeValDoc.node/kind doc
    // - SomeValDoc.text "This comments SOME_VALUE."
    // - @+3SOME_VALUE defines SomeValue

    // This comments SOME_VALUE.
    SOME_VALUE,

    // - AnotherValDoc documents AnotherValue
    // - AnotherValDoc.node/kind doc
    // - AnotherValDoc.text "This documents {@link [ANOTHER_VALUE]}. "
    // - @+3ANOTHER_VALUE defines AnotherValue

    /** This documents {@link ANOTHER_VALUE}. */
    ANOTHER_VALUE;
  }

  // - ToStringDoc documents ToString
  // - ToStringDoc.node/kind doc
  // - ToStringDoc.text "This documents {@link [#toString()]}. "
  // - @+3toString defines/binding ToString

  /** This documents {@link #toString()}. */
  @Override
  public String toString() {
    return "null";
  }

  // - @+4compareTo defines/binding CompareTo

  /** This documents {@link #compareTo(Comments)} over {@link Override}. */
  @Override
  public int compareTo(Comments o) {
    return 0;
  }
  // - OverrideDoc documents CompareTo
  // - OverrideDoc.node/kind doc
  // - OverrideDoc param.0 _Override
  // - OverrideDoc.text "This documents {@link #compareTo(Comments)} over {@link [Override]}. "

  // - @+3fooFunction defines/binding FooFunction

  // This documents FooFunction
  void fooFunction() {
    return;
  }
  // - FooFunctionDoc documents FooFunction
  // - FooFunctionDoc.node/kind doc
  // - FooFunctionDoc.text "This documents FooFunction"
}
