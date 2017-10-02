package pkg;

// Checks that appropriate name nodes are emitted for classes.

//- @Names defines/binding N
//- N named NName = vname("pkg.Names", "", "", "", "jvm")
//- NName.node/kind name
public class Names {

  //- @Inner defines/binding NI
  //- NI named NIName = vname("pkg.Names$Inner", "", "", "", "jvm")
  //- NIName.node/kind name
  private static class Inner {
    //- @Innerception defines/binding NII
    //- NII named NIIName = vname("pkg.Names$Inner$Innerception", "", "", "", "jvm")
    //- NIIName.node/kind name
    private static class Innerception { // is this still funny?
    }
  }

  //- @Generic defines/binding GenericAbs
  //- GenericAbs.node/kind abs
  //- GenericClass childof GenericAbs
  //- GenericClass.node/kind record
  //- GenericClass named GenericClassName = vname("pkg.Names$Generic", "", "", "", "jvm")
  //- GenericClassName.node/kind name
  static class Generic<T> {}

  static void func() {
    //- @Local defines/binding LocalClass
    //- !{ LocalClass named LocalClassName }
    class Local {};
  }
}
