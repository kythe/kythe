// Tests for a class in the default package. All name nodes should be prefixed with a ".", denoting
// the default package.

//- @DefaultPackage defines Class
//- Class named vname(".DefaultPackage","","","","java")
//- !{ Class childof AnyPackage
//-    AnyPackage.node/kind package }
public class DefaultPackage {

  //- @CONSTANT defines ConstantField
  //- ConstantField named vname(".DefaultPackage.CONSTANT","","","","java")
  public static final String CONSTANT = "constantly unused... :-(";

  //- @main defines MainMethod
  //- MainMethod named vname(".DefaultPackage.main(java.lang.String[])","","","","java")
  public static void main(String[] args) {}
}
