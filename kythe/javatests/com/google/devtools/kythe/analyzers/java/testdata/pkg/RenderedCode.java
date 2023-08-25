package pkg;

// - @RenderedCode defines/binding RenderedCode
// - RenderedCode.code/rendered/qualified_name "pkg.RenderedCode"
public final class RenderedCode {
  // - @RenderedCode defines/binding Constructor
  // - Constructor.code/rendered/qualified_name "pkg.RenderedCode.RenderedCode"
  // - Constructor.code/rendered/signature "RenderedCode()"
  // - Constructor.code/rendered/callsite_signature "RenderedCode()"
  private RenderedCode() {}

  // - @Inner defines/binding Inner
  // - Inner.code/rendered/qualified_name "pkg.RenderedCode.Inner"
  // - Inner.code/rendered/signature "Inner<T, U>"
  // - Inner tparam.0 T
  // - T.code/rendered/qualified_name "pkg.RenderedCode.Inner.T"
  public static class Inner<T, U> {}
}
