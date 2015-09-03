//- @pkg ref PkgPackage
//- PkgPackage.node/kind package
package pkg;

//- @"java.util" ref UtilPackage
//- UtilPackage.node/kind package
//- @List ref ListI
import java.util.List;
//- @"java.util" ref UtilPackage
import java.util.LinkedList;

public class Imports {
  //- @List ref ListI
  public static void forAll(List<String> lst) {}
}
