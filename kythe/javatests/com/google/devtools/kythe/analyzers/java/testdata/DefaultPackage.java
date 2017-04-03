// Tests for a class in the default package. All name nodes should be prefixed with a ".", denoting
// the default package.

//- @DefaultPackage defines/binding Class
//- !{ Class childof AnyPackage
//-    AnyPackage.node/kind package }
public class DefaultPackage {

  //- @CONSTANT defines/binding ConstantField
  public static final String CONSTANT = "constantly unused... :-(";

  //- @main defines/binding MainMethod
  public static void main(String[] args) {}
}
