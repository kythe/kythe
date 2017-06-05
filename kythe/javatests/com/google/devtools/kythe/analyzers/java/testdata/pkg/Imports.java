//- @pkg ref PkgPackage
//- PkgPackage.node/kind package
package pkg;

//- @CASE_INSENSITIVE_ORDER ref/imports StringConst
//- @String ref String
import static java.lang.String.CASE_INSENSITIVE_ORDER;
//- @hashCode ref/imports HashCodeMethod
import static java.util.Objects.hashCode;

//- @staticMethod ref/imports StaticBool
//- @staticMethod ref/imports StaticInt
import static pkg.StaticMethods.staticMethod;

//- @"java.util" ref UtilPackage
//- UtilPackage.node/kind package
//- @List ref/imports ListI
import java.util.List;
//- @"java.util" ref UtilPackage
import java.util.LinkedList;

public class Imports {
  //- ListI childof ListAbs
  //- @List ref ListAbs
  public static void forAll(List<String> lst) {
    //- @String ref String
    //- @CASE_INSENSITIVE_ORDER ref StringConst
    System.err.println(String.CASE_INSENSITIVE_ORDER);
  }
}

//- String.node/kind record
