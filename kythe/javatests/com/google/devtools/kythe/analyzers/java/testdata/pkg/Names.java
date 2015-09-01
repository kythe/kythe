//- File.node/kind file
//- File named vname("Names.java","","","","java")

//- @pkg ref Package
//- Package.node/kind package
//- Package named vname("pkg","","","","java")
package pkg;

// Checks that appropriate name nodes are emitted for simple cases

//- @Names defines C
//- C named vname("pkg.Names","","","","java")
public class Names {

  //- @Inner defines I
  //- I named vname("pkg.Names.Inner","","","","java")
  private static class Inner {
    //- @Inner defines ICtor
    //- ICtor named vname("pkg.Names.Inner.Inner()","","","","java")
    //- ICtor typed ICtorType
    //- ICtorType named vname("()","","","","java")
    private Inner() {}

    //- @Inner defines ICtorStr
    //- ICtorStr named vname("pkg.Names.Inner.Inner(java.lang.String)","","","","java")
    //- ICtorStr typed ICtorStrType
    //- ICtorStrType named vname("(java.lang.String)","","","","java")
    private Inner(String s) {}

    //- @Innerception defines Innerception
    //- Innerception named vname("pkg.Names.Inner.Innerception","","","","java")
    private static class Innerception { // is this still funny?
      //- @WE_MUST_GO_DEEPER defines Punchline
      //- Punchline named vname("pkg.Names.Inner.Innerception.WE_MUST_GO_DEEPER","","","","java")
      private static final int WE_MUST_GO_DEEPER = 42;
    }
  }

  //- @func defines F
  //- F named vname("pkg.Names.func(int,java.lang.String)","","","","java")
  //- F typed FType
  //- FType named vname("int(int,java.lang.String)","","","","java")
  //- @arg0 defines P0
  //- P0 named vname("pkg.Names.func(int,java.lang.String)#arg0","","","","java")
  //- @arg1 defines P1
  //- P1 named vname("pkg.Names.func(int,java.lang.String)#arg1","","","","java")
  private static int func(int arg0, String arg1) {
    //- @local defines L
    //- L named vname("pkg.Names.func(int,java.lang.String).{}0#local","","","","java")
    int local = 10;

    //- @l2 defines L2
    //- L2 named vname("pkg.Names.func(int,java.lang.String).{}0#l2","","","","java")
    int l2 = 2;

    return 0;
  }

  //- @classField defines ClassField
  //- ClassField named vname("pkg.Names.classField","","","","java")
  //- @int ref IntType
  //- IntType named vname("int","","","","java")
  private static int classField;

  //- @instanceField defines InstanceField
  //- InstanceField named vname("pkg.Names.instanceField","","","","java")
  private int instanceField;

  //- @varArgsFunc defines VarArgsFunc
  //- VarArgsFunc named vname("pkg.Names.varArgsFunc(int...)","","","","java")
  //- VarArgsFunc typed VarArgsFuncType
  //- VarArgsFuncType named vname("void(int[])","","","","java")
  private static void varArgsFunc(int... varArgsParam) {}

  //- @Generic defines GenericClass
  //- GenericClass.node/kind record
  //- GenericClass named vname("pkg.Names.Generic","","","","java")
  //- @"Generic<T>" defines GenericAbs
  //- GenericAbs.node/kind abs
  //- GenericAbs named vname("pkg.Names.Generic<T>","","","","java")
  static class Generic<T> {
    //- @T ref TVar
    //- TVar named vname("pkg.Names.Generic~T","","","","java")
    T get() { return null; }
  }

  //- @gFunc defines GFunc
  //- GFunc typed GFuncType
  //- GFuncType named vname("pkg.Names.Generic<java.lang.String>()","","","","java")
  //- @"Generic<String>" ref GType
  //- GType named vname("pkg.Names.Generic<java.lang.String>","","","","java")
  static Generic<String> gFunc() { return null; }

  //- @"Generic<String[]>[]" ref GenericStringArrayType
  //- GenericStringArrayType named vname("pkg.Names.Generic<java.lang.String[]>[]","","","","java")
  //- @"String[]" ref StringArrayType
  //- StringArrayType named vname("java.lang.String[]","","","","java")
  static final Generic<String[]>[] arry = null;
}
