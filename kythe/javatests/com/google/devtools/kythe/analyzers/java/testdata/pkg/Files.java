package pkg;

//- @Files defines/binding FilesClass
public class Files {

  //- @"錨" defines/binding UV
  int 錨;
  //- Anchor=vname(_,"kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","java") defines/binding UV
  //- Anchor.node/kind anchor
  //- Anchor.loc/start 112
  //- Anchor.loc/end 115

  //- @Inner defines/binding InnerClass
  //- InnerClass childof FilesClass
  public static class Inner {}

  //- @staticMethod defines/binding _StaticMethod
  public static void staticMethod() {}

  //- @CONSTANT defines/binding _ConstantMember
  public static final int CONSTANT = 42;

  // Ensure this private member does not affect the class node across compilations.
  private int PRIVATE_MEMBER = -42;

  //- @OtherDecl defines/binding _ODecl
  enum OtherDecl {}

  //- @Inter defines/binding InterAbs
  //- InterAbs.node/kind abs
  //- Inter childof InterAbs
  //- Inter.node/kind interface
  interface Inter<T> {}
}

//- File=vname("","kythe","","kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Files.java","")
//-   .node/kind file
//- File.text/encoding "UTF-8"
