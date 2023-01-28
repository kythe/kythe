//- @pkg ref Package
//- Package.node/kind package
package pkg;

@SuppressWarnings({"unused", "UnicodeInCode"})
//- @Files defines/binding FilesClass
public class Files {

  //- @"錨" defines/binding UV
  static final int 錨 = 0;
  //- Anchor=vname(_,"kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","java") defines/binding UV
  //- Anchor.node/kind anchor
  //- Anchor.loc/start 223
  //- Anchor.loc/end 226

  //- @Inner defines/binding InnerClass
  //- InnerClass childof FilesClass
  public static class Inner {}

  //- @staticMethod defines/binding StaticMethod
  //- StaticMethod.node/kind function
  public static void staticMethod() {}

  //- @CONSTANT defines/binding ConstantMember
  //- ConstantMember.node/kind variable
  public static final int CONSTANT = 42;

  //- @INSTANCE defines/binding InstanceMember
  //- InstanceMember.node/kind variable
  public static final Inner INSTANCE = new Inner();

  // Ensure this private member does not affect the class node across compilations.
  private int privateMember = -42;

  //- @OtherDecl defines/binding ODecl
  //- ODecl.node/kind sum
  enum OtherDecl {}

  //- @Inter defines/binding Inter
  //- Inter.node/kind interface
  interface Inter<T> {}
}

//- FilesClass named JVMFiles=vname(_, _, _, _, "jvm")
//- JVMFiles.node/kind record

//- File=vname("","kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","")
//-   .node/kind file
//- File.text/encoding "UTF-8"
