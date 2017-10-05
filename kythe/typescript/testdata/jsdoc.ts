export {}

//- @:8"singleLine" defines/binding VarSingleLine
//- VarSingleLineDoc documents VarSingleLine
//- VarSingleLineDoc.node/kind doc
//- VarSingleLineDoc.text "Single-line doc."
/** Single-line doc. */
var singleLine;

//- @:19"multiLine" defines/binding VarMultiLine
//- VarMultiLineDoc documents VarMultiLine
//- VarMultiLineDoc.node/kind doc
//- VarMultiLineDoc.text "Multi-line doc.\n  spaced in.\nEOT"
/**
 * Multi-line doc.
 *   spaced in.
 * EOT
 */
const multiLine = 3;

//- @:26"Class" defines/binding Class
//- ClassDoc documents Class
//- ClassDoc.node/kind doc
//- ClassDoc.text "Class doc."
/** Class doc. */
class Class {
//- @:32"property" defines/binding Property
//- PropertyDoc documents Property
//- PropertyDoc.node/kind doc
//- PropertyDoc.text "Property doc."
  /** Property doc. */
  private property: string;

//- @:39"method" defines/binding Method
//- MethodDoc documents Method
//- MethodDoc.node/kind doc
//- MethodDoc.text "Method doc."
  /** Method doc. */
  method() {}

//- @:49"method2" defines/binding Method2
//- Method2Doc documents Method2
//- Method2Doc.node/kind doc
//- Method2Doc.text "Multi line doc.\n  Indented."
  /**
   * Multi line doc.
   *   Indented.
   */
  method2() {}
}

//- @:57"myFunc" defines/binding Function
//- FunctionDoc documents Function
//- FunctionDoc.node/kind doc
//- FunctionDoc.text "Function doc."
/** Function doc. */
function myFunc() {}
