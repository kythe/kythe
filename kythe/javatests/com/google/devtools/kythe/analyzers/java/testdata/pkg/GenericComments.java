package pkg;

// After #1501 is fixed, only the abs nodes should have documents/code.

//- @+2GenericComments defines/binding GClass
/** C */
public class GenericComments<T> {
  //- GClassDoc documents GClass
  //- GClassDoc.text "C "

  //- @+2genericMethod defines/binding GFunction
  /** M */
  public static <T> T genericMethod(T x) { return x; }

  //- GFunction.node/kind function
  //- GFunctionDoc documents GFunction
  //- GFunctionDoc.text "M "
}
