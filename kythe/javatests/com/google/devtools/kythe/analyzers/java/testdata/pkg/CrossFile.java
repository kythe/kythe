package pkg;

// Check that nodes unify across file/compilation boundaries

import static pkg.Files.Inner;

public class CrossFile {
  //- @Files ref FilesClass
  Files f1;

  //- @Files ref FilesClass
  //- @Inner ref InnerClass
  Files.Inner f2;

  //- @OtherDecl ref ODecl
  OtherDecl f3;

  //- @Inter ref Inter
  Inter i;

  public static void main(String[] args) {
    //- @staticMethod ref StaticMethod
    Files.staticMethod();
    //- @CONSTANT ref ConstantMember
    System.out.println(Files.CONSTANT);
  }
}
