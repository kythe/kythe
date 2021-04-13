package pkg;

@SuppressWarnings("unused")
public class Visibility {

  //- @privMember defines/binding PrivateMember
  private int privMember;

  //- @defMember defines/binding DefaultMember
  int defMember;

  //- @protMember defines/binding ProtectedMember
  protected int protMember;

  //- @pubMember defines/binding PublicMember
  public int pubMember;

  //- @privMethod defines/binding PrivateMethod
  private int privMethod() {
    return 0;
  }

  //- @defMethod defines/binding DefaultMethod
  int defMethod() {
    return 0;
  }

  //- @protMethod defines/binding ProtectedMethod
  protected int protMethod() {
    return 0;
  }

  //- @pubMethod defines/binding PublicMethod
  public int pubMethod() {

    //- @local defines/binding LocalVar
    int local = 0;
    return local;
  }

  //- @PrivClass defines/binding PrivateClass
  private class PrivClass {}

  //- @DefClass defines/binding DefaultClass
  class DefClass {}

  //- @ProtClass defines/binding ProtectedClass
  protected class ProtClass {}

  //- @PubClass defines/binding PublicClass
  public class PubClass {}
}

//- PrivateMember.visibility private
//- DefaultMember.visibility package
//- ProtectedMember.visibility protected
//- PublicMember.visibility public

//- PrivateMethod.visibility private
//- DefaultMethod.visibility package
//- ProtectedMethod.visibility protected
//- PublicMethod.visibility public

//- PrivateClass.visibility private
//- DefaultClass.visibility package
//- ProtectedClass.visibility protected
//- PublicClass.visibility public

//- !{ LocalVar.visibility _ }
