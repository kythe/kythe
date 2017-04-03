//- File.node/kind file

//- @pkg ref Package
//- Package.node/kind package
package pkg;

// Checks that appropriate name nodes are emitted for simple cases

//- @Names defines/binding C
public class Names {

  //- @Inner defines/binding I
  private static class Inner {
    //- @Inner defines/binding ICtor
    //- ICtor typed ICtorType
    private Inner() {}

    //- @Inner defines/binding ICtorStr
    //- ICtorStr typed ICtorStrType
    private Inner(String s) {}

    //- @Innerception defines/binding Innerception
    private static class Innerception { // is this still funny?
      //- @WE_MUST_GO_DEEPER defines/binding Punchline
      private static final int WE_MUST_GO_DEEPER = 42;
    }
  }

  //- @func defines/binding F
  //- F typed FType
  //- @arg0 defines/binding P0
  //- @arg1 defines/binding P1
  private static int func(int arg0, String arg1) {
    //- @local defines/binding L
    int local = 10;

    //- @l2 defines/binding L2
    int l2 = 2;

    return 0;
  }

  //- @classField defines/binding ClassField
  //- @int ref IntType
  private static int classField;

  //- @instanceField defines/binding InstanceField
  private int instanceField;

  //- @varArgsFunc defines/binding VarArgsFunc
  //- VarArgsFunc typed VarArgsFuncType
  private static void varArgsFunc(int... varArgsParam) {}

  //- @Generic defines/binding GenericAbs
  //- GenericAbs.node/kind abs
  //- GenericClass childof GenericAbs
  //- GenericClass.node/kind record
  static class Generic<T> {
    //- @T ref TVar
    T get() {
      return null;
    }
  }

  //- @gFunc defines/binding GFunc
  //- GFunc typed GFuncType
  //- @"Generic<String>" ref GType
  static Generic<String> gFunc() {
    return null;
  }

  //- @"Generic<String[]>[]" ref GenericStringArrayType
  //- @"String[]" ref StringArrayType
  static final Generic<String[]>[] arry = null;
}
