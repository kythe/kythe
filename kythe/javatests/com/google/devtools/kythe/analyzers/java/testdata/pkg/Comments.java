//- @pkg ref Package
package pkg;

//- @+7"java.lang.Integer" ref/doc IntegerClass
//- @+6String ref/doc StringClass
//- @+5Inner ref/doc InnerClass
//- @+5pkg ref/doc Package
//- @#0+7Comments defines/binding CommentsClass

/**
 * This is a Javadoc comment with links to {@link String}, {@link java.lang.Integer}, {@link Inner},
 * and {@link pkg}. It references {@value IM_A_LONG}.
 */
@SuppressWarnings({"unused", "MultiVariableDeclaration"})
public class Comments implements Comparable<Comments> {

  //- @+8IM_A_LONG defines/binding LongConstant
  //- @+6long ref/doc LongBuiltin
  //- LongBuiltin.node/kind tbuiltin
  //- LongDocNode.node/kind doc
  //- LongDocNode.subkind "javadoc"
  //- LongDocNode documents LongConstant
  //- LongDocNode.text "This is a {@link [long]} with a value of {@value}. "
  /** This is a {@link long} with a value of {@value}. */
  public static final long IM_A_LONG = 4;

  public static final int BLAH80 = 4;

  //- DocNode.node/kind doc
  //- DocNode.subkind "javadoc"
  //- DocNode documents CommentsClass
  //- DocNode param.0 StringClass
  //- DocNode param.1 IntegerClass
  //- DocNode param.2 InnerClass
  //- DocNode param.3 Package
  //- DocNode.text " This is a Javadoc comment with links to {@link [String]}, {@link [java.lang.Integer]}, {@link [Inner]},\n and {@link [pkg]}. It references {@value [IM_A_LONG]}.\n"

  // - @fieldOne defines/binding _FieldOne
  private static int fieldOne; // inline comment here

  //- !{_ documents FieldTwo}
  //- @+3fieldTwo defines/binding FieldTwo

  // fieldTwo represents the universe
  private static String fieldTwo;

  //- @+3fieldThree defines/binding FieldThree
  //- @+3fieldFour defines/binding FieldFour

  private static int fieldThree; // EOL comment
  private static int fieldFour;

  //- !{_ documents FieldThree}
  //- !{_ documents FieldFour }

  //- @+3fieldFive defines/binding FieldFive
  //- @+2fieldSix defines/binding FieldSix

  private static int fieldFive, fieldSix; // EOL comment both

  //- !{_ documents FieldFive}
  //- !{_ documents FieldSix}

  //- @+4fieldSeven defines/binding FieldSeven
  //- @+3fieldEight defines/binding FieldEight

  // above comment both
  private static int fieldSeven, fieldEight;

  //- !{_ documents FieldSeven}
  //- !{_ documents FieldEight}

  //- !{_ documents InnerClass}
  //- @+3Inner defines/binding InnerClass

  // This comments but does not document the Inner class.
  public static class Inner {}

  //- !{_ documents InnerI}
  //- @+4InnerI defines/binding InnerI

  /* This comments the InnerI interface. */
  // a second, weirdly-placed comment
  static interface InnerI {} // this also comments the interface

  //- @+3InnerE ref/doc InnerE
  //- @+3InnerE defines/binding InnerE

  /** This comments the {@link InnerE} enum. */
  static enum InnerE {

    //- InnerEDoc documents InnerE
    //- InnerEDoc.node/kind doc
    //- InnerEDoc param.0 InnerE
    //- InnerEDoc.text "This comments the {@link [InnerE]} enum. "

    //- !{_ documents SomeValue}
    //- @+3SOME_VALUE defines SomeValue

    // This comments SOME_VALUE.
    SOME_VALUE,

    //- AnotherValDoc documents AnotherValue
    //- AnotherValDoc.node/kind doc
    //- AnotherValDoc.text "This documents {@link [ANOTHER_VALUE]}. "
    //- @+3ANOTHER_VALUE defines AnotherValue

    /** This documents {@link ANOTHER_VALUE}. */
    ANOTHER_VALUE;
  }

  //- ToStringDoc documents ToString
  //- ToStringDoc.node/kind doc
  //- ToStringDoc.text "This documents {@link [#toString()]}. "
  //- @+4toString defines/binding ToString

  /** This documents {@link #toString()}. */
  @Override
  public String toString() {
    return "null";
  }

  //- @+4compareTo defines/binding CompareTo

  /** This documents {@link #compareTo(Comments)} over {@link Override}. */
  @Override
  public int compareTo(Comments o) {
    return 0;
  }
  //- OverrideDoc documents CompareTo
  //- OverrideDoc.node/kind doc
  //- OverrideDoc param.0 _Override
  //- OverrideDoc.text "This documents {@link [#compareTo(Comments)]} over {@link [Override]}. "

  //- @+3fooFunction defines/binding FooFunction

  // This documents FooFunction
  void fooFunction() {
    return;
  }
  //- !{_ documents FooFunction}
}
