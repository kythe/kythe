package pkg;

// Check that nodes unify across file/compilation boundaries

//- @CONSTANT ref/imports JvmConstantMember
import static pkg.Files.CONSTANT;
//- @Inner ref/imports JvmInnerClass
import static pkg.Files.Inner;
//- @staticMethod ref/imports JvmStaticMethod
import static pkg.Files.staticMethod;

import pkg.Files.Inter;
import pkg.Files.OtherDecl;

//- ConstantMember generates JvmConstantMember
//- FilesClass generates JvmFilesClass
//- InnerClass generates JvmInnerClass
//- Inter generates JvmInter
//- ODecl generates JvmODecl
//- StaticMethod generates JvmStaticMethod

//- @pkg ref/doc Package
/** Tests JVM references within the {@link pkg} package.*/
public class JvmCrossFile {
  //- @Files ref JvmFilesClass
  Files f1;

  //- @Files ref JvmFilesClass
  //- @Inner ref JvmInnerClass
  Files.Inner f2;

  //- @OtherDecl ref JvmODecl
  OtherDecl f3;

  //- @Inter ref JvmInter
  Inter i;

  //- @InterSub defines/binding InterSub
  //- InterSub.node/kind interface
  //- InterSub extends JvmInter
  interface InterSub extends Inter {}

  public static void main(String[] args) {
    //- @staticMethod ref JvmStaticMethod
    Files.staticMethod();
    //- @staticMethod ref JvmStaticMethod
    staticMethod();
    //- @CONSTANT ref JvmConstantMember
    System.out.println(Files.CONSTANT);
  }
}
