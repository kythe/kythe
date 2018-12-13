package pkg;

//- @CASE_INSENSITIVE_ORDER ref/imports StringConst
//- @String ref String
import static java.lang.String.CASE_INSENSITIVE_ORDER;
//- @hashCode ref/imports _HashCodeMethod
//- @"java.util" ref UtilPackage
import static java.util.Objects.hashCode;

//- @staticMethod ref/imports StaticBool
//- @staticMethod ref/imports StaticInt
import static pkg.StaticMethods.staticMethod;

//- @member ref/imports MemberFunc
//- !{ @member ref/imports PrivateMember }
import static pkg.StaticMethods.member;

//- @staticMember ref/imports StaticMemberFunc
//- @staticMember ref/imports PackageStaticMemberFunc
//- !{ @staticMember ref/imports PrivateStaticMember
//-    @staticMember ref/imports ProtectedStaticMemberFunc }
import static pkg.StaticMethods.staticMember;

//- @"java.util" ref UtilPackage
//- UtilPackage.node/kind package
//- @List ref/imports ListI
import java.util.List;
//- @"java.util" ref UtilPackage
import java.util.LinkedList;
//- @Entry ref/imports MapEntryClass
//- @Map ref MapClass
//- @"java.util" ref UtilPackage
import java.util.Map.Entry;

public class Imports {
  //- ListI childof ListAbs
  //- @List ref ListAbs
  public static void forAll(List<String> lst) {
    //- @String ref String
    //- @CASE_INSENSITIVE_ORDER ref StringConst
    System.err.println(String.CASE_INSENSITIVE_ORDER);
    //- @Entry ref MapEntryClass
    //- @Map ref MapClass
    //- @"java.util" ref UtilPackage
    System.err.println(java.util.Map.Entry.class);
    //- @Entry ref MapEntryClass
    System.err.println(Entry.class);
  }
}

//- String.node/kind record
