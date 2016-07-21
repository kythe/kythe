package pkg;

// Check that nodes unify across file/compilation boundaries

//- @CONSTANT ref/imports ConstantMember
import static pkg.Files.CONSTANT;
//- @Inner ref/imports InnerClass
import static pkg.Files.Inner;
//- @staticMethod ref/imports StaticMethod
import static pkg.Files.staticMethod;

// Make sure wildcard imports don't crash.
import static pkg.Files.*;
import pkg.*;

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
    //- @staticMethod ref StaticMethod
    staticMethod();
    //- @CONSTANT ref ConstantMember
    System.out.println(Files.CONSTANT);
  }
}
