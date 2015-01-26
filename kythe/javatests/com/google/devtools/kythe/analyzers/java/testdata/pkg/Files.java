package pkg;

//- @Files defines FilesClass
public class Files {

  //- @"錨" defines UV
  int 錨;
  //- Anchor defines UV
  //- Anchor.node/kind anchor
  //- Anchor.loc/start 96
  //- Anchor.loc/end 99

  //- @Inner defines InnerClass
  //- InnerClass childof FilesClass
  public static class Inner {}

  //- @staticMethod defines StaticMethod
  public static void staticMethod() {}

  //- @CONSTANT defines ConstantMember
  public static final int CONSTANT = 42;
}
//- Anchor childof File =
//-   vname(_,"kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","java")
//- FilesClass childof File

//- File.node/kind file
//- File.text/encoding "UTF-8"

//- @OtherDecl defines ODecl
//- ODecl childof File
enum OtherDecl {}

//- @Inter defines Inter
//- Inter childof File
interface Inter {}
