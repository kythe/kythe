// - @pkg ref Package
// - Package.code/rendered/signature "package pkg"
package pkg;

import java.util.List;

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

    // - @genericList defines/binding GenericList
    // - GenericList.code/rendered/signature "List<T> genericList"
    List<T> genericList;
  }

  // - @CONSTANT defines/binding Constant
  // - Constant.code/rendered/signature "public static final String CONSTANT"
  public static final String CONSTANT = "blah";

  // - @arry defines/binding Arry
  // - Arry.code/rendered/signature "private int[] arry"
  private int[] arry;

  // - @wildcardList defines/binding WildcardList
  // - WildcardList.code/rendered/signature "List<?> wildcardList"
  // - WildcardList typed WildcardListType
  // - WildcardListType param.1 Wildcard
  // - Wildcard.code/rendered/signature "?"
  List<?> wildcardList;

  // - @extendsBoundedList defines/binding ExtendsBoundedList
  // - ExtendsBoundedList.code/rendered/signature "List<? extends RenderedCode> extendsBoundedList"
  List<? extends RenderedCode> extendsBoundedList;

  // - @superBoundedList defines/binding SuperBoundedList
  // - SuperBoundedList.code/rendered/signature "List<? super Inner<?, ?>> superBoundedList"
  List<? super Inner<?, ?>> superBoundedList;

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
