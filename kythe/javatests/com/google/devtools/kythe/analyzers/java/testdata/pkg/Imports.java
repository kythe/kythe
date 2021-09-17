package pkg;

//- @CASE_INSENSITIVE_ORDER ref/imports StringConst
//- @String ref String
import static java.lang.String.CASE_INSENSITIVE_ORDER;
//- @isNull ref/imports IsNullMethod
//- @"java.util" ref UtilPackage
import static java.util.Objects.isNull;

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
import java.util.ArrayList;
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

    //- @isNull ref IsNullMethod
    boolean unused1 = isNull(CASE_INSENSITIVE_ORDER);
    boolean unused2 = isNull(new ArrayList<>());

    //- @staticMethod ref StaticBool
    staticMethod(member() == staticMember());
  }
}

//- String.node/kind record
