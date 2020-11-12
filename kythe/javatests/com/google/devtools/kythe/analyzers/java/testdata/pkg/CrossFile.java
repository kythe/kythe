package pkg;

// Check that nodes unify across file/compilation boundaries

//- @CONSTANT ref/imports ConstantMember
import static pkg.Files.CONSTANT;
//- @staticMethod ref/imports StaticMethod
import static pkg.Files.staticMethod;

//- @Inner ref/imports InnerClass
import pkg.Files.Inner;

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

  //- @Inner ref InnerClass
  Inner in;

  //- @Inter ref Inter
  Inter i;

  //- @Exception ref Exception
  //- Exception named _JVMException=vname(_, _, _, _, "jvm")
  public static void main(String[] args) throws Exception {
    //- @staticMethod ref StaticMethod
    Files.staticMethod();
    //- @staticMethod ref StaticMethod
    staticMethod();
    //- @CONSTANT ref ConstantMember
    System.out.println(Files.CONSTANT);
    //- @CONSTANT ref ConstantMember
    System.out.println(CONSTANT);
  }
}
