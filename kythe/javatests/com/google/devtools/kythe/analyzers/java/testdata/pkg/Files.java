package pkg;

//- @Files defines/binding FilesClass
public class Files {

  //- @"錨" defines/binding UV
  int 錨;
  //- Anchor defines/binding UV
  //- Anchor.node/kind anchor
  //- Anchor.loc/start 112
  //- Anchor.loc/end 115

  //- @Inner defines/binding InnerClass
  //- InnerClass childof FilesClass
  public static class Inner {}

  //- @staticMethod defines/binding StaticMethod
  public static void staticMethod() {}

  //- @CONSTANT defines/binding ConstantMember
  public static final int CONSTANT = 42;

  // Ensure this private member does not affect the class node across compilations.
  private int PRIVATE_MEMBER = -42;
}
//- Anchor childof File =
//-   vname("","kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","")

//- File.node/kind file
//- File.text/encoding "UTF-8"

//- @OtherDecl defines/binding ODecl
enum OtherDecl {}

//- @Inter defines/binding Inter
interface Inter {}
