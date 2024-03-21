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
  //- ODecl named JVMODecl=vname(_, _, _, _, "jvm")
  //- JVMODecl.node/kind sum
  //- JVMODecl.subkind enumClass
  OtherDecl f3;

  //- @Inner ref InnerClass
  Inner in;

  //- @Inter ref Inter
  //- Inter named JVMInter=vname(_, _, _, _, "jvm")
  //- JVMInter.node/kind interface
  Inter i;

  //- @Exception ref Exception
  //- Exception named JVMException=vname(_, _, _, _, "jvm")
  //- JVMException.node/kind record
  //- JVMException.subkind class
  public static void main(String[] args) throws Exception {
    //- @staticMethod ref StaticMethod
    Files.staticMethod();
    //- @staticMethod ref StaticMethod
    staticMethod();
    //- @CONSTANT ref ConstantMember
    System.out.println(Files.CONSTANT);
    //- @CONSTANT ref ConstantMember
    System.out.println(CONSTANT);

    throw new Exception();
  }
}
