package pkg;

// After #1501 is fixed, only the abs nodes should have documents/code.

//- @+2GenericComments defines/binding GAbsClass
/** C */
public class GenericComments<T> {
  //- GAbsClass.node/kind abs
  //- GClass childof GAbsClass
  //- GClassDoc documents GClass
  //- GClassDoc.text "C "
  //- GClassDoc documents GAbsClass
  //- GClass.code GCCode
  //- GAbsClass.code GCCode

  //- @+2GenericMethod defines/binding GAbsFunction
  /** M */
  public static <T> T GenericMethod(T x) { return x; }

  //- GAbsFunction.node/kind abs
  //- GFunction childof GAbsFunction
  //- GFunctionDoc documents GFunction
  //- GFunctionDoc.text "M "
  //- GFunctionDoc documents GAbsFunction
  //- GFunction.code GFCode
  //- GAbsFunction.code GFCode
}
