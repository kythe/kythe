package pkg;

@SuppressWarnings("unused")
final class NativeMethods {
  // - @f defines/binding MethodFID
  // - MethodFID named vname("Java_pkg_NativeMethods_f__ID","kythe","","","csymbol")
  // - !{ MethodFID named vname("Java_pkg_NativeMethods_f","kythe","","","csymbol") }
  native int f(int x, double j);

  // - @f defines/binding MethodFIO
  // - MethodFIO named vname("Java_pkg_NativeMethods_f__ILjava_lang_Object_2",
  // -     "kythe","","","csymbol")
  // - !{ MethodFIO named vname("Java_pkg_NativeMethods_f","kythe","","","csymbol") }
  native int f(int x, Object o);

  // - @g defines/binding MethodG
  // - MethodG named vname("Java_pkg_NativeMethods_g__I","kythe","","","csymbol")
  // - MethodG named vname("Java_pkg_NativeMethods_g","kythe","","","csymbol")
  native int g(int x);
}
