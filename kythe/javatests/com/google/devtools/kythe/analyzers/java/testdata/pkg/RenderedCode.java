// - @pkg ref Package
// - Package.code/rendered/signature "package pkg"
package pkg;

// - @RenderedCode defines/binding RenderedCode
// - RenderedCode.code/rendered/signature "public final class RenderedCode"
// - RenderedCode.code/rendered/qualified_name "pkg.RenderedCode"
public final class RenderedCode {
  // - @RenderedCode defines/binding Constructor
  // - Constructor.code/rendered/qualified_name "pkg.RenderedCode.RenderedCode"
  // - Constructor.code/rendered/signature "private RenderedCode()"
  // - Constructor.code/rendered/callsite_signature "RenderedCode()"
  private RenderedCode() {}

  // - @Inner defines/binding Inner
  // - Inner.code/rendered/qualified_name "pkg.RenderedCode.Inner"
  // - Inner.code/rendered/signature "public static class Inner<T, U>"
  // - Inner tparam.0 T
  // - T.code/rendered/qualified_name "pkg.RenderedCode.Inner.T"
  public static class Inner<T, U> {}

  // - @CONSTANT defines/binding Constant
  // - Constant.code/rendered/signature "public static final String CONSTANT"
  public static final String CONSTANT = "blah";

  // - @E defines/binding E
  // - E.code/rendered/signature "protected enum E"
  protected enum E {
    // - @VALUE defines/binding Value
    // - Value.code/rendered/signature "E VALUE"
    VALUE
  }

  // - @I defines/binding I
  // - I.code/rendered/signature "interface I"
  interface I {}
}
