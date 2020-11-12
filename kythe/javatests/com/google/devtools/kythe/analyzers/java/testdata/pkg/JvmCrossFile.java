package pkg;

// Check that nodes unify across file/compilation boundaries

//- @CONSTANT ref/imports ConstantMember
import static pkg.Files.CONSTANT;
//- @staticMethod ref/imports StaticMethod
import static pkg.Files.staticMethod;
//- @INSTANCE ref/imports InstanceMember
import static pkg.Files.INSTANCE;

//- @Inner ref/imports InnerClass
import pkg.Files.Inner;
import pkg.Files.Inter;
import pkg.Files.OtherDecl;

//- ConstantMember named JvmConstantMember
//- FilesClass named JvmFilesClass
//- InnerClass named JvmInnerClass
//- Inter named JvmInter
//- ODecl named JvmODecl
//- StaticMethod named JvmStaticMethod
//- InstanceMember named JvmInstanceMember

//- ConstantMember generates JvmConstantMember
//- FilesClass generates JvmFilesClass
//- InnerClass generates JvmInnerClass
//- Inter generates JvmInter
//- ODecl generates JvmODecl
//- StaticMethod generates JvmStaticMethod
//- InstanceMember generates JvmInstanceMember

//- @pkg ref/doc Package
/** Tests JVM references within the {@link pkg} package.*/
public class JvmCrossFile {
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

  //- @InterSub defines/binding InterSub
  //- InterSub.node/kind interface
  //- InterSub extends Inter
  interface InterSub extends Inter {}

  public static void main(String[] args) {
    //- @staticMethod ref StaticMethod
    Files.staticMethod();
    //- @staticMethod ref StaticMethod
    staticMethod();
    //- @CONSTANT ref ConstantMember
    System.out.println(CONSTANT);
    //- @INSTANCE ref InstanceMember
    System.out.println(INSTANCE);
  }
}
