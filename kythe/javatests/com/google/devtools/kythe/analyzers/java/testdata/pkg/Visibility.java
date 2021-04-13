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
    return 0;
  }

  //- @PrivClass defines/binding PrivateClass
  private class PrivClass { }

  //- @DefClass defines/binding DefaultClass
  class DefClass { }

  //- @ProtClass defines/binding ProtectedClass
  protected class ProtClass { }

  //- @PubClass defines/binding PublicClass
  public class PubClass {}
}

//- PrivateMember.visibility private
//- !{ DefaultMember.visibility _ }
//- ProtectedMember.visibility protected
//- PublicMember.visibility public

//- PrivateMethod.visibility private
//- !{ DefaultMethod.visibility _}
//- ProtectedMethod.visibility protected
//- PublicMethod.visibility public

//- PrivateClass.visibility private
//- !{ DefaultClass.visibility _ }
//- ProtectedClass.visibility protected
//- PublicClass.visibility public
