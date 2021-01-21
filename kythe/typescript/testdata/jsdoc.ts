export {}

//- @+5"singleLine" defines/binding VarSingleLine
//- VarSingleLineDoc documents VarSingleLine
//- VarSingleLineDoc.node/kind doc
//- VarSingleLineDoc.text "Single-line doc."
/** Single-line doc. */
var singleLine;

//- @+9"multiLine" defines/binding VarMultiLine
//- VarMultiLineDoc documents VarMultiLine
//- VarMultiLineDoc.node/kind doc
//- VarMultiLineDoc.text "Multi-line doc.\n  spaced in.\nEOT"
/**
 * Multi-line doc.
 *   spaced in.
 * EOT
 */
const multiLine = 3;

//- @+5"Class" defines/binding Class
//- ClassDoc documents Class
//- ClassDoc.node/kind doc
//- ClassDoc.text "Class doc."
/** Class doc. */
class Class {
//- @+5"property" defines/binding Property
//- PropertyDoc documents Property
//- PropertyDoc.node/kind doc
//- PropertyDoc.text "Property doc."
  /** Property doc. */
  private property: string;

//- @+5"method" defines/binding Method
//- MethodDoc documents Method
//- MethodDoc.node/kind doc
//- MethodDoc.text "Method doc."
  /** Method doc. */
  method() {}

//- @+8"method2" defines/binding Method2
//- Method2Doc documents Method2
//- Method2Doc.node/kind doc
//- Method2Doc.text "Multi line doc.\n  Indented."
  /**
   * Multi line doc.
   *   Indented.
   */
  method2() {}
}

//- @+5"myFunc" defines/binding Function
//- FunctionDoc documents Function
//- FunctionDoc.node/kind doc
//- FunctionDoc.text "Function doc."
/** Function doc. */
function myFunc() {}

interface String {
//- @+6"fontsize" defines/binding FontSize
//- FontSize.tag/deprecated "The <font> element has been removed in HTML5.\nPrefer using CSS properties."
  /**
   * @deprecated The <font> element has been removed in HTML5.
   *             Prefer using CSS properties.
   */
  fontsize: (size: number) => string;
//- @+3"big" defines/binding Big
//- Big.tag/deprecated ""
  /** @deprecated */
  big: () => string;
}

//- @+3"PromiseAlias" defines/binding PromiseAlias
//- PromiseAlias.tag/deprecated "dont use me"
/** @deprecated dont use me */
type PromiseAlias<T> = Promise<T>;

//- @+3"Foo" defines/binding IFoo
//- IFoo.tag/deprecated "use Bar"
/** @deprecated use Bar */
interface Foo {}

//- @+3"depFunc" defines/binding DepFunc
//- !{ DepFunc.tag/deprecated "is deprecated" }
/**  @deprecated is deprecated */
function depFunc() {}
