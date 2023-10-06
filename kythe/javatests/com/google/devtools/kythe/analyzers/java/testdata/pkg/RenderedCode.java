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

  // - @create defines/binding Create
  // - Create.code/rendered/qualified_name "pkg.RenderedCode.create"
  // - Create.code/rendered/signature "public static RenderedCode create()"
  // - Create.code/rendered/callsite_signature "create()"
  public static RenderedCode create() {
    return null;
  }

  // - @Inner defines/binding Inner
  // - Inner.code/rendered/qualified_name "pkg.RenderedCode.Inner"
  // - Inner.code/rendered/signature "public static class Inner<T, U>"
  // - Inner tparam.0 T
  // - T.code/rendered/qualified_name "pkg.RenderedCode.Inner.T"
  public static class Inner<T, U> {
    // - @create defines/binding CreateInner
    // - CreateInner.code/rendered/qualified_name "pkg.RenderedCode.Inner.create"
    // - CreateInner.code/rendered/signature "public static <A, B> Inner<A, B> create()"
    // - CreateInner.code/rendered/callsite_signature "create()"
    public static <A, B> Inner<A, B> create() {
      return null;
    }
  }

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
