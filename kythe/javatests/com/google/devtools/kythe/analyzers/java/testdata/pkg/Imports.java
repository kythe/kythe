//- @pkg ref PkgPackage
//- PkgPackage.node/kind package
package pkg;

//- @CASE_INSENSITIVE_ORDER ref/imports StringConst
//- @String ref String
import static java.lang.String.CASE_INSENSITIVE_ORDER;
//- @hashCode ref/imports HashCodeMethod
import static java.util.Objects.hashCode;
// TODO(schroederc): handle overloaded static method imports

//- @"java.util" ref UtilPackage
//- UtilPackage.node/kind package
//- @List ref/imports ListI
import java.util.List;
//- @"java.util" ref UtilPackage
import java.util.LinkedList;

public class Imports {
  //- @List ref ListI
  public static void forAll(List<String> lst) {
    //- @String ref String
    //- @CASE_INSENSITIVE_ORDER ref StringConst
    System.err.println(String.CASE_INSENSITIVE_ORDER);
  }
}

//- String.node/kind record
//- String named vname("java.lang.String","","","","java")
